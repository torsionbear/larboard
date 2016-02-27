#include "Scene.h"

#include <algorithm>
#include <stack>

using std::string;
using std::make_unique;
using std::array;

namespace core {

Scene::Scene() {
    _staticModelGroup = make_unique<StaticModelGroup>();
}

auto Scene::Load() -> void {
    if (nullptr != _staticModelGroup) {
        _staticModelGroup->Load();
    }
    if (nullptr != _skyBox) {
        _skyBox->Load();
    }
    if (nullptr != _terrain) {
        _terrain->Load(_cameras.front()->GetSightDistance());
    }
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

auto Scene::CreateSkyBox(std::string const & filename) -> void {
    _skyBox = make_unique<SkyBox>(filename);
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
        if (currentNode->IsLeaf()) {
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

}