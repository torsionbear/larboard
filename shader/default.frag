#version 430 core

struct Textures {
	sampler2D diffuseTexture;
}; 

struct PointLight {
	vec4 position;
	vec4 color;
	vec4 attenuation;
};

struct DirectionalLight {
	vec4 position;
	vec4 color;
	vec4 attenuation;
};

struct SpotLight {
	vec4 position;
	vec4 color;
	vec4 attenuation;
};

layout (std140, binding = 3) uniform Lights {
	SpotLight spotLights[50];
	PointLight pointLights[50];
	DirectionalLight directionalLights[10];
	int directionalLightCount;
	int pointLightCount;
	int spotLightCount;
} lights;

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	vec4 viewPosition;
} camera;

layout (std140,  binding = 2) uniform Material {
    vec4 diffuse;
    vec4 specular;
	vec4 emissive;
	float ambientIntensity;
    float shininess;
	float transparency;
} material;

uniform Textures textures;

in vec4 fragPosition;
in vec4 fragNormal;
in vec2 fragTexCoord;
layout (location = 0)  out vec4 fragColor;

vec4 viewDirection = normalize(camera.viewPosition - fragPosition);

vec4 processLights();
vec4 processPointLight(PointLight light);

void main()
{
	fragColor = processLights();
	//fragColor = texture(textures.diffuseTexture, fragTexCoord) * max(dot(viewDirection, fragNormal), 0.0) * material.shininess * 10.0;
}

vec4 processLights() {
	vec4 result;
    for(int i = 0; i < lights.pointLightCount; i++) {
		result += processPointLight(lights.pointLights[i]);
	}
	return result;
}

vec4 processPointLight(PointLight light) {
    // Attenuation
    float distance = length(light.position - fragPosition);
	if(distance > light.attenuation[3]) {
		return vec4(0, 0, 0, 1);
	}
    float attenuation = 1.0f / (light.attenuation[0] + light.attenuation[1] * distance + light.attenuation[2] * (distance * distance));

    vec4 lightDirection = normalize(light.position - fragPosition);
    // Diffuse shading
    float diff = max(dot(fragNormal, lightDirection), 0.0);
    // Specular shading
    vec4 reflectDirection = reflect(-lightDirection, fragNormal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
    // Combine results
    vec4 diffuse = light.color * diff * vec4(texture(textures.diffuseTexture, fragTexCoord));
    vec4 specular = light.color * spec * material.specular;
    return (diffuse + specular) * attenuation;
}