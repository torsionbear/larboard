#pragma once

#include <memory>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <dxgidebug.h>

#include <wrl.h>

#include "RenderWindow.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "FencedCommandQueue.h"
#include "SwapChainRenderTargets.h"

#include "core/Shape.h"

namespace d3d12RenderSystem {

using Microsoft::WRL::ComPtr;

struct Vertex {
    float position[3];
    float color[4];
};

class RenderSystem {
public:
    ~RenderSystem();
public:
    auto GetResourceManager() -> ResourceManager * {
        return _resourceManager.get();
    }
    auto GetRenderer() -> Renderer * {
        return _renderer.get();
    }
    auto GetRenderWindow() -> RenderWindow & {
        return _renderWindow;
    }
    auto Init(unsigned int width, unsigned int height) -> void;
    auto RegisterStaticModelGroup(core::StaticModelGroup & staticModelGroup) -> void {
        for (auto & shape : staticModelGroup.GetShapes()) {
            if (shape->GetMaterial()->GetTransparency() > 0) {
                _translucentShapes.push_back(shape.get());
            } else {
                _shapes.push_back(shape.get());
            }
        }
        for (auto & mesh : staticModelGroup.GetMeshes()) {
            _meshes.push_back(mesh.get());
        }
        for (auto & model : staticModelGroup.GetModels()) {
            _models.push_back(model.get());
        }
        for (auto & material : staticModelGroup._materials) {
            _materials.push_back(material.get());
        }
        for (auto & texture : staticModelGroup._textures) {
            _textures.push_back(texture.get());
        }
        _shadowCasterAabb = staticModelGroup.GetBvh()->GetRoot()->GetAabb();
    }
    auto RegisterSkyBox(core::SkyBox * skyBox) -> void {
        _skyBox = skyBox;
    }
    auto RegisterTerrain(core::Terrain * terrain) -> void {
        _terrain = terrain;
    }
    auto RegisterCamera(core::Camera * camera) -> void {
        _camera = camera;
    }
    auto RegisterAmbientLight(core::AmbientLight * ambientLight) -> void {
        _ambientLights.push_back(ambientLight);
    }
    auto RegisterDirectionalLight(core::DirectionalLight * directionalLight) -> void {
        _directionalLights.push_back(directionalLight);
    }
    auto RegisterPointLight(core::PointLight * pointLight) -> void {
        _pointLights.push_back(pointLight);
    }
    auto RegisterSpotLight(core::SpotLight * spotLight) -> void {
        _spotLights.push_back(spotLight);
    }
    auto Load() -> void {
        _renderer->AllocateDescriptorHeap(
            1,
            _meshes.size(),
            _models.size(),
            _textures.size(),
            _materials.size(),
            _skyBox == nullptr ? 0 : 1,
            _terrain == nullptr ? 0 : 1,
            SrvRegisterConvention::Count);
        _resourceManager->LoadBegin();

        if (_skyBox != nullptr) {
            _resourceManager->LoadSkyBox(_skyBox);
        }
        if (_terrain != nullptr) {
            _resourceManager->LoadTerrain(_terrain);
        }
        _resourceManager->LoadCamera(_camera, 1);
        _resourceManager->LoadMeshes(_meshes.data(), _meshes.size(), sizeof(core::Vertex));
        _resourceManager->LoadModels(_models.data(), _models.size());
        _resourceManager->LoadMaterials(_materials.data(), _materials.size());
        _resourceManager->LoadDdsTexture(_textures.data(), _textures.size());
        _resourceManager->LoadShadowCastingLight(_directionalLights.data(), _directionalLights.size());
        _resourceManager->LoadLight(
            _ambientLights.data(), _ambientLights.size(),
            _directionalLights.data(), _directionalLights.size(),
            _pointLights.data(), _pointLights.size(),
            _spotLights.data(), _spotLights.size());
        _renderer->Prepare();
        _renderer->CreateShadowMapBundle(_directionalLights.front(), _shapes.data(), _shapes.size());
        _renderer->CreateSkyBoxBundle(_skyBox);
        _renderer->CreateTerrainBundle(_terrain);
        _renderer->CreateTranslucentBundle(_translucentShapes.data(), _translucentShapes.size());
        _renderer->CreateShapeBundle(_shapes.data(), _shapes.size());
        _resourceManager->LoadEnd();
    }
    auto Draw() -> void {
        _renderer->DrawBegin();
        if (!_directionalLights.empty()) {
            _renderer->DrawShadowMap();
        }
        _renderer->Draw(_camera, _skyBox, _terrain, _shapes.data(), _shapes.size(), _directionalLights.front());
        _renderer->DrawTranslucent();
        _renderer->DrawEnd();
    }
    auto Update() -> void {
        _resourceManager->PrepareResource();
        _resourceManager->UpdateViewpoint(_camera);

        if (!_directionalLights.empty()) {
            auto shadowCastingLight = _directionalLights.front();
            shadowCastingLight->ComputeShadowMappingVolume(_camera, _shadowCasterAabb);
            _resourceManager->UpdateViewpoint(shadowCastingLight);
        }
        if (_terrain != nullptr) {
            _resourceManager->UpdateTerrain(_terrain, _camera);
        }
    }
private:
    auto EnableDebugLayer() -> void;
    auto static CreateFactory() -> ComPtr<IDXGIFactory1>;
    auto static CreateDevice(IDXGIFactory1 * factory) -> ComPtr<ID3D12Device>;

private:
    std::unique_ptr<ResourceManager> _resourceManager;
    std::unique_ptr<Renderer> _renderer;
    RenderWindow _renderWindow;
#if defined(_DEBUG)
    ComPtr<IDXGIDebug1> _dxgiDebug1;
#endif

    std::vector<core::Shape *> _shapes;
    std::vector<core::Shape *> _translucentShapes;
    core::SkyBox * _skyBox;
    core::Terrain * _terrain;
    core::Camera * _camera;

    std::vector<core::Mesh *> _meshes;
    std::vector<core::Model *> _models;
    std::vector<core::Material *> _materials;
    std::vector<core::Texture *> _textures;
    std::vector<core::AmbientLight *> _ambientLights;
    std::vector<core::DirectionalLight *> _directionalLights;
    std::vector<core::PointLight *> _pointLights;
    std::vector<core::SpotLight *> _spotLights;
    core::Aabb _shadowCasterAabb;
};

}