#pragma once

#include <memory>
#include <map>

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
	X3dReader(std::string pathname)
		: _pathName(pathname) {
	}
public:
	// when exporting x3d from blender:
	// 1. export Normals
	// 2. use blender coordinate system, e.g. Y Forward, Z up.
	auto Read(core::Scene * scene) -> void;
private:
	auto ReadIndexedFaceSet(IndexedFaceSet const& indexedFaceSet, core::StaticModelGroup & staticModelGroup) -> core::Mesh<core::Vertex> *;
	auto ReadIndexedTriangleSet(IndexedTriangleSet const& indexedTriangleSet, core::StaticModelGroup & staticModelGroup) -> core::Mesh<core::Vertex> *;
    auto ReadTransform(Transform const& transform, core::StaticModelGroup & staticModelGroup) -> core::Movable *;
	auto ReadScene(Scene const& scene) -> void;
	auto ReadX3d(X3d const& x3d) -> void;
	auto ReadShapes(vector<Shape*> const& shapes, core::StaticModelGroup & staticModelGroup) -> core::Model *;
	auto ReadMaterial(Material const& material, core::StaticModelGroup & staticModelGroup) -> core::Material *;
	auto ReadImageTexture(ImageTexture const& imageTexture, core::StaticModelGroup & staticModelGroup) -> std::vector<core::Texture *>;
	auto ReadViewpoint(Viewpoint const& viewpoint)->core::Camera *;
	auto ReadPointLight(PointLight const& pointLight)->core::PointLight *;
	auto ReadDirectionalLight(DirectionalLight const& directionalLight)->core::DirectionalLight *;
	auto ReadSpotLight(SpotLight const& spotLight) -> core::SpotLight *;
private:
    core::Scene * _scene;
	boost::filesystem::path _pathName;
	X3dParser _x3dParser;
    std::map<std::string, core::Material *> _materials;
    std::map<std::string, std::vector<core::Texture *>> _imageTextures;
};

}