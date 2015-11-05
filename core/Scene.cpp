#include "Scene.h"

#include <algorithm>
#include <stack>

#include <GL/glew.h>

using std::string;
using std::make_unique;

namespace core {

Scene::Scene() {
    // todo: send in resourceManager by argument
    _resourceManager = make_unique<ResourceManager>();
    _staticModelGroup = make_unique<StaticModelGroup>(_resourceManager.get());
}

auto Scene::CreateCamera() -> Camera * {
	_cameras.push_back(make_unique<Camera>());
	return _cameras.back().get();
}

auto Scene::CreateAmbientLight() -> AmbientLight * {
    _ambientLights.push_back(make_unique<AmbientLight>());
    return _ambientLights.back().get();
}

auto Scene::CreateDirectionalLight() -> DirectionalLight * {
	_directionalLights.push_back(make_unique<DirectionalLight>());
	return _directionalLights.back().get();
}

auto Scene::CreatePointLight() -> PointLight * {
	_pointLights.push_back(make_unique<PointLight>());
	return _pointLights.back().get();
}

auto Scene::CreateSpotLight() -> SpotLight * {
	_spotLights.push_back(make_unique<SpotLight>());
	return _spotLights.back().get();
}

auto Scene::Stage(Movable * movable) -> void {
	movable->AttachTo(_root);
}

auto Scene::Unstage(Movable * movable) -> void {
	movable->DetachFrom();
}

auto Scene::GetActiveCamera() const -> Camera * {
	return _cameras.front().get();
}

auto Scene::Picking(Ray & ray) -> bool {
    auto ret = false;
    auto bvhRoot = _staticModelGroup->GetBvh()->GetRoot();
    auto nodeStack = std::stack<BvhNode *>{};
    nodeStack.push(bvhRoot);
    while (!nodeStack.empty()) {
        auto currentNode = nodeStack.top();
        nodeStack.pop();
        auto length = currentNode->GetAabb().IntersectRay(ray);
        if (length < 0) {
            continue;
        }
        if (currentNode->RightChild() != nullptr) {
            nodeStack.push(currentNode->RightChild());
        }
        if (currentNode->LeftChild() != nullptr) {
            nodeStack.push(currentNode->LeftChild());
        }
        if(currentNode->IsLeaf()) {
            auto shapes = currentNode->GetShapes();
            for (auto shape : shapes) {
                auto length = shape->GetAabb().IntersectRay(ray);
                if (length > 0) {
                    ray.length = length;
                    ret = true;
                }
            }
        }
    }
    return ret;
}

auto Scene::ToggleBackFace() -> void {
	_renderBackFace = !_renderBackFace;
	_renderBackFace ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
}

auto Scene::ToggleWireframe() -> void {
	_wireframeMode = !_wireframeMode;
	_wireframeMode ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

auto Scene::PrepareForDraw() -> void {

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// 0. setup ubo
	InitCameraData();
	LoadLightData();
    _staticModelGroup->PrepareForDraw();
    _staticModelGroup->GetBvh()->PrepareForDraw(*_resourceManager);

	// todo: sort shapes according to: 1. shader priority; 2. vbo/vao
}

auto Scene::Draw() -> void {
	auto error = glGetError();

	// 0. feed model independent data (camera) to shader via ubo
	LoadCameraData();
	UseCameraData(_cameras.front().get());

    _staticModelGroup->Draw();
    _staticModelGroup->GetBvh()->Draw();

	error = glGetError();
}

// store all camera's data in _cameraUbo. 
// if there's not need to split screen for multiple player, 
// maybe we should store only 1 camera's data in _cameraUbo
auto Scene::InitCameraData() -> void {
	glGenBuffers(1, &_cameraUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, _cameraUbo);
	glBufferData(GL_UNIFORM_BUFFER, Camera::ShaderData::Size() * _cameras.size(), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto Scene::LoadCameraData() -> void {
	auto cache = vector<unsigned char>(Camera::ShaderData::Size() * _cameras.size());
	auto offset = 0;
	for (auto & camera : _cameras) {
		auto * p = reinterpret_cast<Camera::ShaderData *>(&cache[offset]);
		*p = camera->GetShaderData();
		camera->SetUboOffset(offset);
		offset += Camera::ShaderData::Size();
	}
	glBindBuffer(GL_UNIFORM_BUFFER, _cameraUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, cache.size(), cache.data());
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto Scene::UseCameraData(Camera const * camera) -> void {
	glBindBufferRange(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Camera), _cameraUbo, camera->GetUboOffset(), sizeof(Camera::ShaderData));
}

auto Scene::LoadLightData() -> void {
	auto data = LightShaderData{};

    data.ambientLight = _ambientLights.front()->GetShaderData();

	data.directionalLightCount = _directionalLights.size();
	assert(data.directionalLightCount <= LightShaderData::MaxDirectionalLightCount);
	for (auto i = 0; i < data.directionalLightCount; ++i) {
		data.directionalLights[i] = _directionalLights[i]->GetShaderData();
	}

	data.pointLightCount = _pointLights.size();
	assert(data.pointLightCount <= LightShaderData::MaxPointLightCount);
	for (auto i = 0; i < data.pointLightCount; ++i) {
		data.pointLights[i] = _pointLights[i]->GetShaderData();
	}

	data.spotLightCount = _spotLights.size();
	assert(data.spotLightCount <= LightShaderData::MaxSpotLightCount);
	for (auto i = 0; i < data.spotLightCount; ++i) {
		data.spotLights[i] = _spotLights[i]->GetShaderData();
	}

	glGenBuffers(1, &_lightUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, _lightUbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(data), &data, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Light), _lightUbo);
}

}