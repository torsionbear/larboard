#pragma once

#include <vector>
#include <memory>
#include <map>

#include "ResourceManager.h"
#include "SceneNode.h"
#include "Camera.h"
#include "Movable.h"
#include "Model.h"
#include "Shape.h"
#include "Camera.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "Bvh.h"
#include "StaticModelGroup.h"

namespace core {

struct LightShaderData {
	enum {
		MaxDirectionalLightCount = 10,
		MaxPointLightCount = 50,
		MaxSpotLightCount = 50,
	};
	DirectionalLight::ShaderData directionalLights[MaxDirectionalLightCount];
	PointLight::ShaderData pointLights[MaxPointLightCount];
	SpotLight::ShaderData spotLights[MaxSpotLightCount];
	int directionalLightCount;
	int pointLightCount;
	int spotLightCount;
};

class Scene {
public:
    Scene();
	Scene(Scene const&) = delete;
	Scene& operator=(Scene const&) = delete;
	~Scene() = default;
public:
	auto CreateCamera() -> Camera *;
	auto CreateDirectionalLight() -> DirectionalLight *;
	auto CreatePointLight() -> PointLight *;
	auto CreateSpotLight() -> SpotLight *;
	auto Stage(Movable *) -> void;
	auto Unstage(Movable *) -> void;

    auto GetActiveCamera() const -> Camera *;
    auto GetStaticModelGroup() -> StaticModelGroup & {
        return *_staticModelGroup;
    }
    auto Picking(Ray & ray) -> bool;

	auto ToggleBackFace() -> void;
	auto ToggleWireframe() -> void;
	auto Draw() -> void;
    auto PrepareForDraw() -> void;

private:
	auto InitCameraData() -> void;
	auto LoadCameraData() -> void;
	auto UseCameraData(Camera const* camera) -> void;
	auto LoadLightData() -> void;

private:
	std::unique_ptr<ResourceManager> _resourceManager;
	Movable _root;
	std::vector<std::unique_ptr<Camera>> _cameras;
	std::vector<std::unique_ptr<PointLight>> _pointLights;
	std::vector<std::unique_ptr<DirectionalLight>> _directionalLights;
	std::vector<std::unique_ptr<SpotLight>> _spotLights;

    std::unique_ptr<StaticModelGroup> _staticModelGroup = nullptr;

	openglUint _cameraUbo;
	openglUint _lightUbo;

	bool _wireframeMode = false;
	bool _renderBackFace = false;
};

}