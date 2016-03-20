#include "CameraController.h"

#include "Scene.h"

namespace core {

auto CameraController::Step() -> void {
    if (!_enable) {
        return;
    }
    auto now = std::chrono::steady_clock::now();
    auto camera = _scene->GetActiveCamera();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastTimePoint).count();
    if (abs(_forwardSpeed) > std::numeric_limits<Float32>::epsilon()
        || abs(_rightSpeed) > std::numeric_limits<Float32>::epsilon()
        || abs(_upwardSpeed) > std::numeric_limits<Float32>::epsilon()) {
        auto forwardDirection = camera->GetForwardDirection();
        forwardDirection(2) = 0;
        Normalize(forwardDirection);
        auto speed = static_cast<core::Vector4f>(forwardDirection * _forwardSpeed + camera->GetRightDirection() * _rightSpeed);
        speed(2) = _upwardSpeed;
        auto displacement = static_cast<core::Vector4f>(speed * static_cast<Float32>(elapsedTime) / 1000.0f);
        auto translatedPosition = _aabb.Translate(camera->GetPosition() + displacement);
        if (!_scene->Intersect(translatedPosition)) {
            camera->Translate(displacement);
        }
    }
    auto position = camera->GetPosition();
    auto terrain = _scene->GetTerrain();
    auto terrainHeight = terrain->GetHeight(core::Vector2f{ position(0), position(1) });
    auto ray = Ray{ position, Vector4f{ 0.0f, 0.0f, -1.0f, 0.0f }, 10.0f };
    auto picked = _scene->Picking(ray);
    auto height = picked ? std::max(terrainHeight, position(2) - ray.length) : terrainHeight;
    // falling down
    auto distanceToGroud = position(2) - _cameraHeight - height;
    if (distanceToGroud > std::numeric_limits<Float32>::epsilon()) {
        _upwardSpeed += elapsedTime / 1000.0f * -9.8f; // gravity acceleration
    } else {
        camera->Translate(core::Vector4f{ 0, 0, -distanceToGroud, 0 });
        _upwardSpeed = 0;
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
