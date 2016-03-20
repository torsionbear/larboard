#pragma once

#include <chrono>

#include "Camera.h"
#include "Aabb.h"

namespace core {

class CameraController {
public:
    CameraController(Scene * scene)
        : _scene(scene) {
        _aabb = Aabb{ Point4f{-0.2f, -0.2f, -0.9f, 1.0f}, Point4f{0.2f, 0.2f, -0.0f, 1.0f} };
    }
public:
    auto Step() -> void;
    auto Enable() -> void {
        _enable = true;
    }
    auto Disable() -> void;
    auto IncreaseForwardSpeed(core::Float32 value) -> void;
    auto IncreaseRightSpeed(core::Float32 value) -> void;
private:
    bool _enable = false;
    Float32 _forwardSpeed = 0;
    Float32 _rightSpeed = 0;
    Float32 _upwardSpeed = 0;
    Aabb _aabb;
    Float32 _cameraHeight = 1.7f;
    Scene * _scene;

    std::chrono::steady_clock::time_point _lastTimePoint;
};

}