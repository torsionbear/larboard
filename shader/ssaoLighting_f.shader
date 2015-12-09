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

struct GBuffer {
	sampler2D diffuseEmissive;
	sampler2D specularShininess;
	sampler2D normal;
	sampler2D occlusion;
	sampler2D depth;
};

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	mat4 projectTransform;
	mat4 viewTransformInverse;
	vec4 viewPosition;
} camera;

uniform GBuffer gBuffer;
uniform int randomVectorTexSize;

in vec2 vTexCoord;
in vec3 viewDirection;

layout (location = 0)  out vec4 fColor;

float screenSpaceDepthToViewSpaceDepth(float screenSpaceDepth) {
	float ndcDepth = screenSpaceDepth * 2 - 1;
	return camera.projectTransform[3][2] / (camera.projectTransform[2][3] * ndcDepth - camera.projectTransform[2][2]);
}

vec4 CalculateViewSpacePosition(float depth) {
	float viewCoordZ =  screenSpaceDepthToViewSpaceDepth(depth);
	return vec4(viewDirection * -viewCoordZ, 1);
}

float GetOcclusion() {
	// blur gBuffer.occlusion by randomVectorTexSize
	vec2 texelSize = 1.0 / textureSize(gBuffer.occlusion, 0);
	float occlusion = 0;
	for(int i = 0; i < randomVectorTexSize; ++i) {
		for(int j = 0; j < randomVectorTexSize; ++j) {
			vec2 offset = (vec2(float(i), float(j)) - randomVectorTexSize * 0.5) * texelSize;
			occlusion += texture(gBuffer.occlusion, vTexCoord + offset).r;
		}
	}
	return occlusion / (randomVectorTexSize * randomVectorTexSize);
}

float DiffuseCoefficient(vec3 normal, vec3 lightDirection) {
	return max(dot(normal, -lightDirection), 0.0);
}

float SpecularCoefficient(vec3 normal, vec3 lightDirection, vec3 viewDirection, float shininess) {
	float cosine = max(dot(viewDirection, reflect(lightDirection, normal)), 0);
	// multiple 128 to x3d shininess. see http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/lighting.html#Lightingmodel
	shininess *= 128;
	return shininess > 0 ? pow(cosine, shininess) : 1; // avoid pow(0, 0)
}

float Attenuation(vec4 attenuation, float distance) {
	if(distance > attenuation[3]) {
		return 0;
	}
	return 1.0f / (attenuation[0] + attenuation[1] * distance + attenuation[2] * (distance * distance));
}

void main()
{
	vec3 position = (camera.viewTransformInverse * CalculateViewSpacePosition(texture(gBuffer.depth, vTexCoord).r)).xyz;
	vec3 viewDirection = normalize(camera.viewPosition.xyz - position);
	
	float occlusion = GetOcclusion();
	vec4 diffuseEmissive = texture(gBuffer.diffuseEmissive, vTexCoord);
	vec4 specularShininess = texture(gBuffer.specularShininess, vTexCoord);
	//fColor = vec4(vec3(1.0 - occlusion), 1.0);	
	vec3 normal = texture(gBuffer.normal, vTexCoord).rgb;

	vec3 ambient = lights.ambientLight.color.rgb * diffuseEmissive.rgb * (1 - occlusion) + diffuseEmissive.rgb * diffuseEmissive.a;
	vec3 diffuse = vec3(0);
	vec3 specular = vec3(0);
    for(int i = 0; i < lights.directionalLightCount; i++) {
		DirectionalLight directionLight = lights.directionalLights[i];
		diffuse += directionLight.color.rgb * diffuseEmissive.rgb * DiffuseCoefficient(normal, directionLight.direction.xyz);
		specular += directionLight.color.rgb * specularShininess.rgb * SpecularCoefficient(normal, directionLight.direction.xyz, viewDirection, specularShininess.a);
	}
    for(int i = 0; i < lights.pointLightCount; i++) {
		PointLight pointLight = lights.pointLights[i];
		float attenuation = Attenuation(pointLight.attenuation, length(pointLight.position.xyz - position));
		vec3 lightDirection = normalize(position - pointLight.position.xyz);
		diffuse += pointLight.color.rgb * diffuseEmissive.rgb * attenuation * DiffuseCoefficient(normal, lightDirection);
		specular += pointLight.color.rgb * specularShininess.rgb * attenuation * SpecularCoefficient(normal, lightDirection, viewDirection, specularShininess.a);
	}
    for(int i = 0; i < lights.spotLightCount; i++) {
		SpotLight spotLight = lights.spotLights[i];
		float attenuation = Attenuation(spotLight.attenuation, length(spotLight.position.xyz - position));
		vec3 lightDirection = normalize(position - spotLight.position.xyz);
		float angle = acos(dot(lightDirection, spotLight.direction.xyz));
		float angleFalloff = 0;
		if(angle < spotLight.cutOffAngle) {
			angleFalloff = angle > spotLight.beamWidth ? (spotLight.cutOffAngle - angle) / (spotLight.cutOffAngle - spotLight.beamWidth) : 1.0;
		}
		diffuse += spotLight.color.rgb * diffuseEmissive.rgb * attenuation * angleFalloff * DiffuseCoefficient(normal, lightDirection);
		specular += spotLight.color.rgb * specularShininess.rgb * attenuation * angleFalloff * SpecularCoefficient(normal, lightDirection, viewDirection, specularShininess.a);
	}
	fColor = vec4(ambient + diffuse + specular, 1);
	//fColor = vec4(specularShininess.a, 0, 0, 1);
}