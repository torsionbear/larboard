#pragma once

#include <vector>
#include <memory>

#include "Texture.h"
#include "Mesh.h"
#include "ShaderProgram.h"

namespace core {

class ResourceManager {
public:
	ResourceManager() = default;
	ResourceManager(ResourceManager const&) = delete;
	ResourceManager& operator=(ResourceManager const&) = delete;

public:
	auto CreateTexture(std::string const&) -> Texture *;
	auto CreateMesh() -> Mesh *;
	auto CreateShaderProgram(std::string const& vertexShaderFile, std::string const& fragmentShaderFile) -> ShaderProgram *;
	auto CreateDefaultShaderProgram() -> ShaderProgram *;

private:
	std::vector<Texture> _textures;
	std::vector<Mesh> _meshes;
	std::vector<ShaderProgram> _shaderProgram;
};

}