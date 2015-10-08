#version 430 core

struct Textures {
	sampler2D diffuseMap;
}; 

struct PointLight {
	vec4 color;
	vec4 position;
	vec4 attenuation;
};

struct DirectionalLight {
	vec4 color;
	vec4 direction;
};

struct SpotLight {
	vec4 color;
	vec4 position;
	vec4 direction;
	vec4 attenuation;
	float beamWidth;
	float cutOffAngle;
	vec2 pad1;	// float pad[2] does not work because array element will round up to size of vec4
};

layout (std140, binding = 3) uniform Lights {
	DirectionalLight directionalLights[10];
	PointLight pointLights[50];
	SpotLight spotLights[50];
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
vec4 processDirectionalLight(DirectionalLight light);
vec4 processPointLight(PointLight light);
vec4 processSpotLight(SpotLight light);

void main()
{
	fragColor = processLights();
	//fragColor = texture(textures.diffuseMap, fragTexCoord) * max(dot(viewDirection, fragNormal), 0.0) * material.ambientIntensity * 3;
}

vec4 processLights() {
	vec4 result;
    for(int i = 0; i < lights.directionalLightCount; i++) {
		result += processDirectionalLight(lights.directionalLights[i]);
	}
    for(int i = 0; i < lights.pointLightCount; i++) {
		result += processPointLight(lights.pointLights[i]);
	}
    for(int i = 0; i < lights.spotLightCount; i++) {
		result += processSpotLight(lights.spotLights[i]);
	}
	return result;
}

vec4 processDirectionalLight(DirectionalLight light) {
    float diffuseCoefficient = max(dot(fragNormal, -light.direction), 0.0);
    vec4 reflectDirection = reflect(light.direction, fragNormal);
    float specularCoefficient = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
	
    vec4 diffuse = light.color * vec4(texture(textures.diffuseMap, fragTexCoord)) * diffuseCoefficient;
    vec4 specular = light.color * material.specular * specularCoefficient ;
    return (specular + diffuse);
}

vec4 processPointLight(PointLight light) {
    float distance = length(light.position - fragPosition);
	if(distance > light.attenuation[3]) {
		return vec4(0, 0, 0, 1);
	}
    float attenuation = 1.0f / (light.attenuation[0] + light.attenuation[1] * distance + light.attenuation[2] * (distance * distance));

    vec4 lightDirection = normalize(fragPosition - light.position);
    float diffuseCoefficient = max(dot(fragNormal, -lightDirection), 0.0);
    vec4 reflectDirection = reflect(lightDirection, fragNormal);
    float specularCoefficient = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
	
    vec4 diffuse = light.color * vec4(texture(textures.diffuseMap, fragTexCoord)) * diffuseCoefficient;
    vec4 specular = light.color * material.specular * specularCoefficient ;
    return (specular + diffuse) * attenuation;
}

vec4 processSpotLight(SpotLight light) {
    float distance = length(light.position - fragPosition);
	if(distance > light.attenuation[3]) {
		return vec4(0, 0, 0, 1);
	}
    float attenuation = 1.0f / (light.attenuation[0] + light.attenuation[1] * distance + light.attenuation[2] * (distance * distance));

    vec4 lightDirection = normalize(fragPosition - light.position);
	float angle = acos(dot(lightDirection, light.direction));
	if(angle > light.cutOffAngle) {
		return vec4(0, 0, 0, 1);
	}
	float angleFalloff = angle > light.beamWidth ? (light.cutOffAngle - angle) / (light.cutOffAngle - light.beamWidth) : 1.0;
    float diffuseCoefficient = max(dot(fragNormal, -lightDirection), 0.0);
    vec4 reflectDirection = reflect(lightDirection, fragNormal);
    float specularCoefficient = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
	
    vec4 diffuse = light.color * vec4(texture(textures.diffuseMap, fragTexCoord)) * diffuseCoefficient;
    vec4 specular = light.color * material.specular * specularCoefficient ;
    return (specular + diffuse) * attenuation * angleFalloff;
}