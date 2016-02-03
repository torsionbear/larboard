#pragma once

#include <vector>
#include <memory>
#include <map>

#include "SceneNode.h"
#include "Camera.h"
#include "Movable.h"
#include "Model.h"
#include "Shape.h"
#include "AmbientLight.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "Skybox.h"
#include "Bvh.h"
#include "StaticModelGroup.h"
#include "terrain.h"

namespace core {

class Scene {
public:
    Scene();
	Scene(Scene const&) = delete;
	Scene& operator=(Scene const&) = delete;
public:
    auto Load() -> void;
	auto CreateCamera() -> Camera *;
    auto CreateAmbientLight() -> AmbientLight *;
    auto CreateDirectionalLight() -> DirectionalLight *;
	auto CreatePointLight() -> PointLight *;
	auto CreateSpotLight() -> SpotLight *;
	auto Stage(Movable *) -> void;
	auto Unstage(Movable *) -> void;

    auto GetActiveCamera() const -> Camera *;
    auto GetStaticModelGroup() -> StaticModelGroup & {
        return *_staticModelGroup;
    }
    auto GetTerrain() -> Terrain * {
        return _terrain.get();
    }
    auto CreateTerrain(std::vector<std::string> && diffuseMapFiles, std::string const& heightMap) -> void;
    auto CreateSkyBox(std::array<std::string, 6> && filenames) -> void;
    auto Picking(Ray & ray) -> bool;

    auto ToggleBvh() -> void;

public:
	Movable _root;
	std::vector<std::unique_ptr<Camera>> _cameras;
    std::vector<std::unique_ptr<AmbientLight>> _ambientLights;
	std::vector<std::unique_ptr<PointLight>> _pointLights;
	std::vector<std::unique_ptr<DirectionalLight>> _directionalLights;
	std::vector<std::unique_ptr<SpotLight>> _spotLights;

    std::unique_ptr<SkyBox> _skyBox = nullptr;
    std::unique_ptr<StaticModelGroup> _staticModelGroup = nullptr;
    std::unique_ptr<Terrain> _terrain = nullptr;

    bool _drawBvh = false;
};

}