#pragma once

#include "renderSystem/RenderWindow.h"
#include "core/scene.h"
#include "core/Matrix.h"

class InputHandler {
    enum Status { none, rotate, pan };
public:
    InputHandler(core::Scene * scene, int width, int height)
        : _scene(scene)
        , _widthInverse(1.0f / static_cast<float>(width))
        , _heightInverse(1.0f / static_cast<float>(height)) {
    }
public:
    auto operator()(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> void;
private:
    auto GetClientSpaceCoordinates(HWND hWnd, WPARAM wParam, LPARAM lParam, bool isScreenSpaceCoordinate) -> core::Vector2f;
private:
    core::Scene * _scene;
    core::Float32 _widthInverse;
    core::Float32 _heightInverse;
    core::Vector2f _lastCoordinate;
    Status _status = none;
    core::Point4f pickPoint = core::Point4f{ 0, 0, 0, 1 };
};