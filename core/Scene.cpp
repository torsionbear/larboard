#include "Scene.h"

#include <algorithm>
#include <stack>

#include "Triangle.h"

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
    auto resultLength = ray.length;
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
                auto distanceToShapeAabb = shape->GetAabb().IntersectRay(ray);
                if (distanceToShapeAabb < 0) {
                    continue;
                }
                auto const& vertex = shape->GetMesh()->GetVertex();
                auto const& index = shape->GetMesh()->GetIndex();
                for (auto i = 0; i < index.size(); i += 3) {
                    auto const& transform = shape->GetModel()->GetTransform();
                    auto index0 = index[i];
                    auto index1 = index[i + 1];
                    auto index2 = index[i + 2];
                    auto triangle = array<Point4f, 3>{
                        transform * Point4f{ vertex[index0].coord(0),vertex[index0].coord(1),vertex[index0].coord(2), 1.0f },
                            transform * Point4f{ vertex[index1].coord(0),vertex[index1].coord(1),vertex[index1].coord(2), 1.0f },
                            transform * Point4f{ vertex[index2].coord(0),vertex[index2].coord(1),vertex[index2].coord(2), 1.0f },
                    };
                    auto distanceToTriangleAabb = Triangle::IntersectRay(ray, triangle);
                    if (distanceToTriangleAabb  > 0) {
                        resultLength = std::min(resultLength, distanceToTriangleAabb);
                        ret = true;
                    }
                }
            }
        }
    }
    ray.length = resultLength;
    return ret;
}

auto Scene::Intersect(Aabb & aabb) -> bool {
    auto ret = false;
    auto bvhRoot = _staticModelGroup->GetBvh()->GetRoot();
    auto nodeStack = std::stack<BvhNode *>{};
    nodeStack.push(bvhRoot);
    while (!nodeStack.empty()) {
        auto currentNode = nodeStack.top();
        nodeStack.pop();
        if (!aabb.IntersectAabb(currentNode->GetAabb())) {
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
                auto const& vertex = shape->GetMesh()->GetVertex();
                auto const& index = shape->GetMesh()->GetIndex();
                for (auto i = 0; i < index.size(); i += 3) {
                    auto const& transform = shape->GetModel()->GetTransform();
                    auto intersected = aabb.IntersectTriangle(std::array<Point4f, 3>{
                        transform * Point4f{ vertex[i].coord(0),vertex[i].coord(1),vertex[i].coord(2), 1.0f },
                            transform * Point4f{ vertex[i + 1].coord(0),vertex[i + 1].coord(1),vertex[i + 1].coord(2), 1.0f },
                            transform * Point4f{ vertex[i + 2].coord(0),vertex[i + 2].coord(1),vertex[i + 2].coord(2), 1.0f},
                    });
                    if (intersected) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

auto Scene::ToggleBvh() -> void {
    _drawBvh = !_drawBvh;
}

}