#version 430 core

struct GBuffer {
	sampler2D color;
	sampler2D normal;
	sampler2D depth;
}; 

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
	mat4 viewProjectTransform;
	mat4 projectTransform;
	mat4 rotationInverse;
	vec4 viewPosition;
} camera;

layout (std140,  binding = 2) uniform Material {
    vec4 diffuseEmissive;
    vec4 specularShininess;
} material;

uniform GBuffer gBuffer;

//in vec2 vPosition;
//in vec4 vragNormal;
in vec2 vTexCoord;
layout (location = 0)  out vec4 fColor;

vec4 visualizeDepth(float depth) {
    float near = 0.1;
    float far = 1000.0;
    float v = 4 * (2.0 * near) / (far + near - depth * (far - near));
	return vec4(vec3(v), 1);
}

void main()
{
	fColor = texture(gBuffer.color, vTexCoord);
	
	//fColor = visualizeDepth(texture(gBuffer.depth, vTexCoord).r);
	//fColor = texture(gBuffer.normal, vTexCoord);
}