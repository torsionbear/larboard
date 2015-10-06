#version 430 core

// 1. glsl expect column major matrix, but our matrix is row major, so we need to specify row_major in shader's interface block layout.
// 2. when using glUniformMatrix4fv() to feed shader with matrix, we need to pass GL_TRUE for the 3rd parameter to transpose our matrix.
// 3. another dirty solution is to always multiply vector to matrix in shader (e.g. v_transformed = v * M)
// see http://stackoverflow.com/questions/17717600/confusion-between-c-and-opengl-matrix-order-row-major-vs-column-major#
layout (std140, row_major, binding = 0) uniform Camera {
	mat4 viewTransform;
	vec4 viewPosition;
} camera;

layout (std140, binding = 1) uniform Transform {
    layout (row_major) mat4 worldTransform;
    layout (row_major) mat4 normalTransform;
} transform;

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec2 vertTexCoord;

out vec4 fragPosition;
out vec4 fragNormal;
out vec2 fragTexCoord;

void main()
{
	fragPosition = transform.worldTransform * vec4(vertPosition, 1);
    gl_Position = camera.viewTransform * fragPosition;
	fragNormal = transform.normalTransform * vec4(vertNormal, 0);
	fragTexCoord = vertTexCoord;
}