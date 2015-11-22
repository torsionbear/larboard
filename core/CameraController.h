#pragma once

#include <chrono>

#include "Camera.h"

namespace core {

class CameraController {
public:
    CameraController(Scene * scene)
        : _scene(scene) {
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
    Scene * _scene;

    std::chrono::steady_clock::time_point _lastTimePoint;
};

}