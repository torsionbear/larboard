#include "Shape.h"

namespace core {
Shape::Shape(Model * model) 
	: _model(model) {
}

auto Shape::SetDiffuse(Vector3f diffuse) -> void {
	_diffuse = diffuse;
}

auto Shape::SetSpecular(Vector3f specular) -> void {
	_specular = specular;
}

auto Shape::SetEmissive(Vector3f emissive) -> void {
	_emissive = emissive;
}

auto Shape::SetAmbientIntensity(Float32 ambientIntensity) -> void {
	_ambientIntensity = ambientIntensity;
}

auto Shape::SetShininess(Float32 shininess) -> void {
	_shininess = shininess;
}

auto Shape::SetTransparency(Float32 transparency) -> void {
	_transparency = transparency;
}

auto Shape::SetMesh(Mesh * mesh) -> void {
	_mesh = mesh;
}

auto Shape::AddTexture(Texture * texture) -> void {
	_textures.push_back(texture);
}

auto Shape::SetShaderProgram(ShaderProgram * shaderProgram) -> void {
	_shaderProgram = shaderProgram;
}

}