#include "Shape.h"

namespace core {
Shape::Shape(Model * model) 
	: _model(model) {
}

auto Shape::SetMaterial(Material * material) -> void {
	_material = material;
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