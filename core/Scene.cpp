#include "Scene.h"

#include <algorithm>
#include <stack>

using std::string;
using std::make_unique;
using std::array;

namespace core {

Scene::Scene(unsigned int width, unsigned int height, ResourceManager * resourceManager, Renderer * renderer)
    : _ssao(width, height) {
    // todo: send in resourceManager by argument
    _resourceManager = resourceManager;
    _renderer = renderer;
    _staticModelGroup = make_unique<StaticModelGroup>(resourceManager, renderer);
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
    _renderer->PrepareForDraw();
    _ssao.PrepareForDraw();

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
        _resourceManager->LoadSkyBoxMesh(_skyBox.get());
        _skyBox->GetCubeMap()->Load();
        _resourceManager->LoadCubeMap(_skyBox->GetCubeMap());
        _skyBox->GetShaderProgram()->SendToCard();
    }

    // terrain
    if (nullptr != _terrain) {
        auto diffuseMap = _terrain->GetDiffuseMap();
        auto heightMap = _terrain->GetHeightMap();
        diffuseMap->Load();
        heightMap->Load();
        _resourceManager->LoadTextureArray(diffuseMap);
        _resourceManager->LoadTexture(heightMap);

        _terrain->SetSightDistance(_cameras.front()->GetSightDistance());
        _resourceManager->LoadTerrain(_terrain.get());
        _terrain->GetShaderProgram()->SendToCard();
        _resourceManager->LoadTerrainSpecialTiles(_terrain->GetSpecialTiles());
    }
	// todo: sort shapes according to: 1. shader priority; 2. vbo/vao

}

auto Scene::Draw() -> void {
    _ssao.BindGBuffer();
    _renderer->DrawBegin();

    // feed model independent data (camera) to shader via ubo
    _resourceManager->UpdateCameraData(_cameras);
    _resourceManager->UseCameraData(_cameras.front().get());

    if (nullptr != _skyBox) {
        _renderer->RenderSkyBox(_skyBox.get());
    }
    if (nullptr != _terrain) {
        _resourceManager->UpdateTerrainTileCoordUbo(_terrain->GetVio(), _terrain->GetTileCoordinate(_cameras.front().get()));
        _renderer->UseTextureArray(_terrain->GetDiffuseMap(), TextureUsage::DiffuseTextureArray);
        _renderer->UseTexture(_terrain->GetHeightMap(), TextureUsage::HeightMap);
        _renderer->DrawTerrain(_terrain.get());
    }
    _staticModelGroup->Draw();
    if (_drawBvh) {
        auto const& aabbs = _staticModelGroup->GetBvh()->GetAabbs();
        for (auto aabb : aabbs) {
            _renderer->RenderAabb(aabb);
        }
    }
    _ssao.SsaoPass();
    _ssao.LightingPass();
}

}