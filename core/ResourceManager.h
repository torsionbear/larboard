#pragma once

#include <vector>
#include <memory>

#include "Mesh.h"

namespace core {

class ResourceManager {
public:
	auto LoadMeshes(std::vector<std::unique_ptr<Mesh>> const& meshes) -> void;

private:
	std::vector<openglUint> _vertexArrayObjects;
	std::vector<openglUint> _vertexBufferObjects;
	std::vector<openglUint> _vertexElementObjects;
};

}