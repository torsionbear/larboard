#pragma once

#include <vector>
#include <memory>

#include "Mesh.h"

namespace core {

class ResourceManager {
public:
	auto LoadMeshes(std::vector<std::unique_ptr<Mesh>> const& meshes) -> void;
    auto LoadBvh(std::vector<Vector3f> const& vertexData, std::vector<unsigned int> const& indexData) -> openglUint;
private:
	std::vector<openglUint> _vertexArrayObjects;
	std::vector<openglUint> _vertexBufferObjects;
	std::vector<openglUint> _vertexElementObjects;
};

}