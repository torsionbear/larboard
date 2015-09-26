#pragma once

#include <memory>

#include "X3d.h"
#include "Transform.h"
#include "IndexedFaceSet.h"
#include "Appearance.h"
#include "core/Scene.h"
#include "core/SceneNode.h"
#include "core/Camera.h"
#include "core/PointLight.h"

using std::make_unique;

namespace x3dParser {

class X3dReader {
public:
	// when exporting x3d from blender:
	// 1. export Normals
	// 2. use blender coordinate system, e.g. Y Forward, Z up.
	static auto Read(std::string const&) -> std::unique_ptr<core::Scene>;
public:
	X3dReader() {
		_scene = make_unique<core::Scene>();
	}
private:
	auto Read(IndexedFaceSet const&) -> core::Mesh *;
    auto Read(Transform const&) -> core::Movable *;
	auto Read(Scene const& scene) ->std::unique_ptr<core::Scene>;
	auto Read(X3d const&) ->std::unique_ptr<core::Scene>;
	auto Read(Shape const&) -> core::Model *;
	auto Read(Material const& material) -> core::Material *;
	auto Read(ImageTexture const&) -> core::Texture*;
	auto Read(Viewpoint const&)->core::Camera *;
	auto Read(PointLight const&) -> core::PointLight *;

private:
	std::unique_ptr<core::Scene> _scene;
};

}