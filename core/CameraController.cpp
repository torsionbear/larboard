#include "CameraController.h"

#include "Scene.h"

namespace core {

auto CameraController::Step() -> void {
    if (!_enable) {
        return;
    }
    auto now = std::chrono::steady_clock::now();
    if (abs(_forwardSpeed) > std::numeric_limits<Float32>::epsilon() || abs(_rightSpeed) > std::numeric_limits<Float32>::epsilon()) {
        auto camera = _scene->GetActiveCamera();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastTimePoint).count();
        auto speed = static_cast<core::Vector4f>(camera->GetForwardDirection() * _forwardSpeed + camera->GetRightDirection() * _rightSpeed);
        auto displacement = static_cast<core::Vector4f>(speed * static_cast<Float32>(elapsedTime) / 1000.0f);
        camera->Translate(displacement);

        auto position = camera->GetPosition();
        auto terrain = _scene->GetTerrain();
        auto height = terrain->GetHeight(core::Vector2f{ position(0), position(1) });

        camera->Translate(core::Vector4f{ 0, 0, height - position(2) + 2, 0 });
    }
    _lastTimePoint = now;
}

auto CameraController::Disable() -> void {
    _enable = false;
    _forwardSpeed = 0;
    _rightSpeed = 0;
}

auto CameraController::IncreaseForwardSpeed(core::Float32 value) -> void {
    _forwardSpeed += value;
}

auto CameraController::IncreaseRightSpeed(core::Float32 value) -> void {
    _rightSpeed += value;
}

}
