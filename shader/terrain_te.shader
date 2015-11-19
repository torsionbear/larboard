#version 430 core

layout (triangles, equal_spacing, ccw) in;

struct Textures {
	sampler2D heightMap;
	sampler2DArray diffuseTextureArray;
}; 

uniform float tileSize;
uniform float sightDistance;
uniform ivec2 gridOrigin;
uniform int gridWidth;
uniform ivec2 heightMapOrigin;
uniform ivec2 heightMapSize;
uniform ivec2 diffuseMapOrigin;
uniform ivec2 diffuseMapSize;

uniform Textures textures;

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewProjectTransform;
	mat4 projectTransform;
	mat4 rotationInverse;
	vec4 viewPosition;
} camera;

//in vec2 tcHeightMapTexCoord[];
in vec2 tcDiffuseMapTexCoord[];

out vec4 tePosition;
out vec3 teDiffuseMapTexCoord;
out vec3 teNormal;

float Height(vec2 position) {
	vec2 heightMapTexCoord = (position / tileSize - heightMapOrigin)  / heightMapSize;
	return texture(textures.heightMap, heightMapTexCoord).x * 30 - 0.6;
}

void main() {
	tePosition = gl_in[0].gl_Position * gl_TessCoord.x + gl_in[1].gl_Position * gl_TessCoord.y + gl_in[2].gl_Position * gl_TessCoord.z;
	
	float height = Height(tePosition.xy);
	vec2 diffuseMapTexCoord = tcDiffuseMapTexCoord[0] * gl_TessCoord.x + tcDiffuseMapTexCoord[1] * gl_TessCoord.y + tcDiffuseMapTexCoord[2] * gl_TessCoord.z;
	teDiffuseMapTexCoord = vec3(diffuseMapTexCoord, (height + 10) / 15);
	
	tePosition.z += height;
	gl_Position = camera.viewProjectTransform * tePosition ;

	// normal
    float step = tileSize / gl_TessLevelInner[0];
    vec3 position_dx = tePosition.xyz + vec3(step, 0, 0);
    vec3 position_dy = tePosition.xyz + vec3(0, step, 0);
    position_dx.z = Height(position_dx.xy);
    position_dy.z = Height(position_dy.xy);
    teNormal = normalize(cross(position_dx - tePosition.xyz, position_dy - tePosition.xyz));
}