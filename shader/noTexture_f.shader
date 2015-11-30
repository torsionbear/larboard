#version 430 core

struct AmbientLight {
	vec4 color;
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
	AmbientLight ambientLight;
	DirectionalLight directionalLights[10];
	PointLight pointLights[50];
	SpotLight spotLights[50];
	int directionalLightCount;
	int pointLightCount;
	int spotLightCount;
} lights;

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	mat4 projectTransform;
	mat4 rotationInverse;
	vec4 viewPosition;
} camera;

layout (std140,  binding = 2) uniform Material {
    vec4 diffuseEmissive;
    vec4 specularShininess;
} material;

in vec4 vPosition;
in vec4 vNormal;
in vec2 vTexCoord;
layout (location = 0)  out vec4 fColor;
layout (location = 1)  out vec4 fNormal;

vec4 viewDirection = normalize(camera.viewPosition - vPosition);

vec4 processLights();
vec4 processAmbientLight(AmbientLight light);
vec4 processDirectionalLight(DirectionalLight light);
vec4 processPointLight(PointLight light);
vec4 processSpotLight(SpotLight light);

void main()
{
	fNormal = vNormal;
	fColor = processLights();
}

vec4 processLights() {
	vec4 result;
	result += processAmbientLight(lights.ambientLight);
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

vec4 processAmbientLight(AmbientLight light) {
    return light.color * material.diffuseEmissive;
}

vec4 processDirectionalLight(DirectionalLight light) {
    float diffuseCoefficient = max(dot(vNormal, -light.direction), 0.0);
    vec4 reflectDirection = reflect(light.direction, vNormal);
    float specularCoefficient = pow(max(dot(viewDirection, reflectDirection), 0.0), material.specularShininess.a);
	
    vec4 diffuse = light.color * vec4(material.diffuseEmissive.rgb, 1) * diffuseCoefficient;
    vec4 specular = light.color * vec4(material.specularShininess.rgb, 1) * specularCoefficient ;
    return (specular + diffuse);
}

vec4 processPointLight(PointLight light) {
    float distance = length(light.position - vPosition);
	if(distance > light.attenuation[3]) {
		return vec4(0, 0, 0, 1);
	}
    float attenuation = 1.0f / (light.attenuation[0] + light.attenuation[1] * distance + light.attenuation[2] * (distance * distance));

    vec4 lightDirection = normalize(vPosition - light.position);
    float diffuseCoefficient = max(dot(vNormal, -lightDirection), 0.0);
    vec4 reflectDirection = reflect(lightDirection, vNormal);
    float specularCoefficient = pow(max(dot(viewDirection, reflectDirection), 0.0), material.specularShininess.a);
	
    vec4 diffuse = light.color * vec4(material.diffuseEmissive.rbg, 1) * diffuseCoefficient;
    vec4 specular = light.color * vec4(material.specularShininess.rgb, 1) * specularCoefficient ;
    return (specular + diffuse) * attenuation;
}

vec4 processSpotLight(SpotLight light) {
    float distance = length(light.position - vPosition);
	if(distance > light.attenuation[3]) {
		return vec4(0, 0, 0, 1);
	}
    float attenuation = 1.0f / (light.attenuation[0] + light.attenuation[1] * distance + light.attenuation[2] * (distance * distance));

    vec4 lightDirection = normalize(vPosition - light.position);
	float angle = acos(dot(lightDirection, light.direction));
	if(angle > light.cutOffAngle) {
		return vec4(0, 0, 0, 1);
	}
	float angleFalloff = angle > light.beamWidth ? (light.cutOffAngle - angle) / (light.cutOffAngle - light.beamWidth) : 1.0;
    float diffuseCoefficient = max(dot(vNormal, -lightDirection), 0.0);
    vec4 reflectDirection = reflect(lightDirection, vNormal);
    float specularCoefficient = pow(max(dot(viewDirection, reflectDirection), 0.0), material.specularShininess.a);
	
    vec4 diffuse = light.color * vec4(material.diffuseEmissive.rgb, 1) * diffuseCoefficient;
    vec4 specular = light.color * vec4(material.specularShininess.rgb, 1) * specularCoefficient ;
    return (specular + diffuse) * attenuation * angleFalloff;
}