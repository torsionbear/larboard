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
    auto RegisterShape(core::Shape * shape) -> void {
        _shapes.push_back(shape);
    }
    auto RegisterSkyBox(core::SkyBox * skyBox) -> void {
        _skyBox = skyBox;
    }
    auto RegisterCamera(core::Camera * camera) -> void {
        _camera = camera;
    }
    auto RegisterMesh(core::Mesh * mesh) -> void {
        _meshes.push_back(mesh);
    }
    auto RegisterModel(core::Model * model) -> void {
        _models.push_back(model);
    }
    auto RegisterMaterial(core::Material * material) -> void {
        _materials.push_back(material);
    }
    auto RegisterTexture(core::Texture * texture) -> void {
        _textures.push_back(texture);
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
            4u);
        _resourceManager->LoadBegin();

        _resourceManager->LoadSkyBox(_skyBox);
        _resourceManager->LoadCamera(_camera, 1);
        _resourceManager->LoadMeshes(_meshes.data(), _meshes.size(), sizeof(core::Vertex));
        _resourceManager->LoadModels(_models.data(), _models.size());
        _resourceManager->LoadMaterials(_materials.data(), _materials.size());
        _resourceManager->LoadDdsTexture(_textures.data(), _textures.size());        
        _resourceManager->LoadLight(
            _ambientLights.data(), _ambientLights.size(),
            _directionalLights.data(), _directionalLights.size(),
            _pointLights.data(), _pointLights.size(),
            _spotLights.data(), _spotLights.size());
        _renderer->Prepare();
        _resourceManager->LoadEnd();
    }
    auto Draw() -> void {
        _renderer->Draw(_camera, _skyBox, _shapes.data(), _shapes.size());
    }
    auto Update() -> void {
        _resourceManager->PrepareResource();
        _resourceManager->UpdateCamera(_camera);
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
    core::SkyBox * _skyBox;
    core::Camera * _camera;

    std::vector<core::Mesh *> _meshes;
    std::vector<core::Model *> _models;
    std::vector<core::Material *> _materials;
    std::vector<core::Texture *> _textures;
    std::vector<core::AmbientLight *> _ambientLights;
    std::vector<core::DirectionalLight *> _directionalLights;
    std::vector<core::PointLight *> _pointLights;
    std::vector<core::SpotLight *> _spotLights;
};

}