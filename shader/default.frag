#version 430 core

struct Textures {
	sampler2D diffuseTexture;
}; 

struct Lights {
	sampler2D data;
	int directionalLightCount;
	int pointLightCount;
	int spotLightCount;
};

layout (std140) uniform Material {
    vec4 diffuse;
    vec4 specular;
	vec4 emissive;
	float ambientIntensity;
    float shininess;
	float transparency;
} material;

uniform vec4 viewPosition;
uniform Textures textures;
//uniform Lights lights;

in vec4 fragPosition;
in vec4 fragNormal;
in vec2 fragTexCoord;
layout (location = 0)  out vec4 fragColor;

vec4 viewDirection = normalize(viewPosition - fragPosition);

/*
vec3 processLights() {
	// point light
	// vec4 (position x, y, z, w) | vec4 (color r, g, b, a) | vec4 (attenuation constant, linear, quadratic, ambientIntensity)
	int lightTexSize = lights.directionalLightCount*3 + lights.pointLightCount*3 + lights.spotLightCount*5;
	vec3 result vec3(0.0);
    for(int i = 0; i < pointLightCount; i++) {
        vec4 position = texture(lights.data, vec2((3*i + 0.5)/lightTexSize, 0));
        vec4 color = texture(lights.data, vec2((3*i + 1 + 0.5)/lightTexSize, 0));
        vec4 attenuation = texture(lights.data, vec2((3*i + 2 + 0.5)/lightTexSize, 0));
		result += processPointLight(position, color, attenuation);
	}
	return result;
}

vec3 processPointLight(vec4 lightPosition, vec4 lightColor, vec4 lightAttenuation) {
    vec4 lightDirection = normalize(lightPosition - fragPosition);
    // Diffuse shading
    float diff = max(dot(fragNormal, lightDirection), 0.0);
    // Specular shading
    vec3 reflectDirection = reflect(-lightDirection, fragNormal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
    // Attenuation
    float distance = length(lightPosition - fragPosition);
    float attenuation = 1.0f / (lightAttenuation[0] + lightAttenuation[1] * distance + lightAttenuation[2] * (distance * distance));
    // Combine results
    vec3 ambient = lightAttenuation[3] * vec3(texture(texture0, fragTexCoord));
    vec3 diffuse = lightColor * diff * vec3(texture(texture0, fragTexCoord));
    vec3 specular = lightColor * spec * material.specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
*/

void main()
{
	//vec3 result = processLights();
    //fragColor = vec4(result, 1.0);
	fragColor = texture(textures.diffuseTexture, fragTexCoord) * max(dot(viewDirection, fragNormal), 0.0) * material.diffuse.r;
}