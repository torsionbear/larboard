#include "Scene.h"

#include <algorithm>
#include <stack>

using std::string;
using std::make_unique;
using std::array;

namespace core {

auto Scene::UpdateScene(ResourceManager * resourceManager, Scene const* scene) -> void {
    //updates
    resourceManager->UpdateCameraData(scene->_cameras);
    resourceManager->UseCameraData(scene->_cameras.front().get());
    if (nullptr != scene->_terrain) {
        resourceManager->UpdateTerrain(scene->_terrain.get(), scene->_cameras.front().get());
    }
}

auto Scene::DrawScene(Renderer * renderer, Scene const * scene) -> void {
    renderer->DrawBegin();

    if (nullptr != scene->_skyBox) {
        renderer->RenderSkyBox(scene->_skyBox.get());
    }
    if (nullptr != scene->_terrain) {
        renderer->DrawTerrain(scene->_terrain.get());
    }
    for (auto const& shape : scene->_staticModelGroup->GetShapes()) {
        renderer->Render(shape.get());
    }
    if (scene->_drawBvh) {
        auto const& aabbs = scene->_staticModelGroup->GetBvh()->GetAabbs();
        for (auto aabb : aabbs) {
            renderer->RenderAabb(aabb);
        }
    }
    renderer->DrawEnd();
}

Scene::Scene(ResourceManager * resourceManager) {
    _resourceManager = resourceManager;
    _staticModelGroup = make_unique<StaticModelGroup>(resourceManager);
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

auto Scene::CreateTerrain(vector<string> && diffuseMapFiles, string const& heightMap) -> void {
    _terrain = make_unique<Terrain>(move(diffuseMapFiles), heightMap);
}

auto Scene::CreateSkyBox(std::array<std::string, 6>&& filenames) -> void {
    _skyBox = make_unique<SkyBox>(move(filenames));
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

auto Scene::ToggleBvh() -> void {
    _drawBvh = !_drawBvh;
}

auto Scene::PrepareForDraw() -> void {

	// setup ubo
    _resourceManager->InitCameraData(Camera::ShaderData::Size() * _cameras.size());
    _resourceManager->InitLightData(_ambientLights, _pointLights, _directionalLights, _spotLights);
    _staticModelGroup->PrepareForDraw();

    // bvh
    auto bvh = _staticModelGroup->GetBvh();    
    auto shaderProgram = bvh->GetShaderProgram();
    shaderProgram->SendToCard();
    auto aabbs = bvh->GetAabbs();
    for (auto aabb : aabbs) {
        aabb->SetShaderProgram(shaderProgram);
    }
    _resourceManager->LoadAabbs(aabbs);

    // sky box
    if (nullptr != _skyBox) {
        _skyBox->Load();        
        _resourceManager->LoadSkyBox(_skyBox.get());
    }

    // terrain
    if (nullptr != _terrain) {
        _terrain->Load(_cameras.front()->GetSightDistance());
        _resourceManager->LoadTerrain(_terrain.get());
    }
	// todo: sort shapes according to: 1. shader priority; 2. vbo/vao
}

}