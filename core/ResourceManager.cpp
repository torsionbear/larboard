#include "ResourceManager.h"

using std::string;
using std::unique_ptr;
using std::make_unique;
using std::move;

namespace core {

auto ResourceManager::CreateTexture(std::string const& filename) -> Texture* {
	// todo: make this factory method creating different textures according to different file type.
	_textures.emplace_back(filename);
	return &_textures.back();
}

auto ResourceManager::CreateMesh() -> Mesh * {
	_meshes.emplace_back();
	return &_meshes.back();
}

auto ResourceManager::CreateShaderProgram(string const& vertexShaderFile, string const& fragmentShaderFile) -> ShaderProgram * {
	_shaderProgram.emplace_back(vertexShaderFile, fragmentShaderFile);
	return &_shaderProgram.back();
}

auto ResourceManager::CreateDefaultShaderProgram() -> ShaderProgram * {
	return CreateShaderProgram("triangles.vert", "triangles.frag");
}

}