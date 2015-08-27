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
	static auto Read(std::string const&) -> std::unique_ptr<core::Scene>;
public:
	X3dReader() {
		_scene = make_unique<core::Scene>();
	}
private:
	auto Read(IndexedFaceSet&) -> core::Mesh *;
    auto Read(Transform&) -> core::Movable *;
	auto Read(Scene & scene) ->std::unique_ptr<core::Scene>;
	auto Read(X3d&) ->std::unique_ptr<core::Scene>;
	auto Read(Shape&) -> core::Model *;
	auto Read(ImageTexture&) -> vector<core::Texture*>;
	auto Read(Viewpoint&)->core::Camera *;
	auto Read(PointLight&) -> core::PointLight *;

private:
	std::unique_ptr<core::Scene> _scene;
};

}