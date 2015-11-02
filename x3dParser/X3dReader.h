#pragma once

#include <memory>

#include <boost/filesystem/path.hpp>

#include "X3dParser.h"
#include "X3d.h"
#include "Transform.h"
#include "IndexedFaceSet.h"
#include "IndexedTriangleSet.h"
#include "Appearance.h"
#include "core/Scene.h"
#include "core/SceneNode.h"
#include "core/Camera.h"
#include "core/PointLight.h"
#include "core/DirectionalLight.h"
#include "core/SpotLight.h"

using std::make_unique;

namespace x3dParser {

class X3dReader {
public:
	X3dReader(std::string pathname, core::Scene * scene)
		: _pathName(pathname)
        , _scene(scene) {
	}
public:
	// when exporting x3d from blender:
	// 1. export Normals
	// 2. use blender coordinate system, e.g. Y Forward, Z up.
	auto Read() -> void;
private:
	auto Read(IndexedFaceSet const& indexedFaceSet)->core::Mesh *;
	auto Read(IndexedTriangleSet const& indexedTriangleSet) -> core::Mesh *;
    auto Read(Transform const& transform) -> core::Movable *;
	auto Read(Scene const& scene) -> void;
	auto Read(X3d const& x3d) -> void;
	auto Read(vector<Shape*> const& shapes) -> core::Model *;
	auto Read(Material const& material) -> core::Material *;
	auto Read(ImageTexture const& imageTexture) -> core::Texture*;
	auto Read(Viewpoint const& viewpoint)->core::Camera *;
	auto Read(PointLight const& pointLight)->core::PointLight *;
	auto Read(DirectionalLight const& directionalLight)->core::DirectionalLight *;
	auto Read(SpotLight const& spotLight) -> core::SpotLight *;

private:
    core::Scene * _scene;
	boost::filesystem::path _pathName;
	X3dParser _x3dParser;
};

}