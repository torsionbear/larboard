#version 430 core

layout (triangles, equal_spacing, ccw) in;

struct Textures {
	sampler2D heightMap;
	sampler2DArray diffuseTextureArray;
};

uniform Textures textures;

layout (std140, binding = 4) uniform Terrain {	
	ivec2 heightMapOrigin;
	ivec2 heightMapSize;
	ivec2 diffuseMapOrigin;
	ivec2 diffuseMapSize;
	float tileSize;
	float sightDistance;	
} terrain;

layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	mat4 projectTransform;
	mat4 viewTransformInverse;
	vec4 viewPosition;
} camera;

in vec2 tcDiffuseMapTexCoord[];
out vec4 tePosition;
out vec3 teDiffuseMapTexCoord;
out vec3 teNormal;

float Height(vec2 position) {
	vec2 heightMapTexCoord = (position / terrain.tileSize - terrain.heightMapOrigin)  / terrain.heightMapSize;
	return texture(textures.heightMap, heightMapTexCoord).x * 30 - 0.6;
}

void main() {
	tePosition = gl_in[0].gl_Position * gl_TessCoord.x + gl_in[1].gl_Position * gl_TessCoord.y + gl_in[2].gl_Position * gl_TessCoord.z;
	
	float height = Height(tePosition.xy);
	vec2 diffuseMapTexCoord = tcDiffuseMapTexCoord[0] * gl_TessCoord.x + tcDiffuseMapTexCoord[1] * gl_TessCoord.y + tcDiffuseMapTexCoord[2] * gl_TessCoord.z;
	teDiffuseMapTexCoord = vec3(diffuseMapTexCoord, (height + 10) / 15);
	
	tePosition.z += height;
	gl_Position = camera.projectTransform * camera.viewTransform * tePosition ;

	// normal
    float step = terrain.tileSize / gl_TessLevelInner[0];
    vec3 position_dx = tePosition.xyz + vec3(step, 0, 0);
    vec3 position_dy = tePosition.xyz + vec3(0, step, 0);
    position_dx.z = Height(position_dx.xy);
    position_dy.z = Height(position_dy.xy);
    teNormal = normalize(cross(position_dx - tePosition.xyz, position_dy - tePosition.xyz));
}