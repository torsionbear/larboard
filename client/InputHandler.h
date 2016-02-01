#pragma once

#include "renderSystem/RenderWindow.h"
#include "core/scene.h"
#include "core/Matrix.h"

class InputHandler {
private:
    enum Status { none, rotate, pan };
public:
    InputHandler(core::ResourceManager * resourceManager, core::Renderer * renderer, core::Scene * scene, core::CameraController * cameraController, int width, int height)
        : _cameraController(cameraController)
        , _resourceManager(resourceManager)
        , _renderer(renderer)
        , _scene(scene)
        , _center{ width / 2, height / 2 }
        , _widthInverse(1.0f / static_cast<float>(width))
        , _heightInverse(1.0f / static_cast<float>(height)) {
    }
public:
    auto operator()(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> void;
private:
    auto GetClientSpaceCoordinates(HWND hWnd, WPARAM wParam, LPARAM lParam, bool isScreenSpaceCoordinate) -> POINT;
private:
    bool _isFpsMode = false;
    core::CameraController * _cameraController;
    core::ResourceManager * _resourceManager;
    core::Renderer * _renderer;
    core::Scene * _scene;
    POINT const _center;
    core::Float32 _widthInverse;
    core::Float32 _heightInverse;
    core::Vector2f _lastCoordinate;
    Status _status = none;
    core::Point4f pickPoint = core::Point4f{ 0, 0, 0, 1 };
};