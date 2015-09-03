#pragma once

#include <memory>

#include "SceneNode.h"

namespace core {

class Movable {
public:
	friend class Scene;

public:
	Movable() = default;
	Movable(Vector3f const& forwardDirection, Vector3f const& upwardDirection)
		: _forwardDirection(forwardDirection) 
		, _upwardDirection(upwardDirection) 
		, _rightwardDirection(_forwardDirection * _upwardDirection) {		
	}
	Movable(Movable const&)
		: _sceneNode(std::make_unique<SceneNode>()) {
	}
	Movable(Movable && other)
		: Movable() {
		swap(*this, other);
	}
	Movable& operator=(Movable rhs) {
		swap(*this, rhs);
		return *this;
	}
	Movable& operator=(Movable && rhs) {
		swap(*this, rhs);
		return *this;
	}
	~Movable() = default;
	friend auto swap(Movable & first, Movable & second) -> void;

public:
	auto Forward(Float32 length) -> void;
	auto Backward(Float32 length) -> void;
	auto Leftward(Float32 length) -> void;
	auto Rightward(Float32 length) -> void;
	auto Upward(Float32 length) -> void;
	auto Downward(Float32 length) -> void;

	auto Translate(Float32, Float32, Float32) -> void;
	auto Rotate(Float32, Float32, Float32, Float32) -> void;
	auto AttachTo(Movable &) -> void;
	auto DetachFrom() -> void;
	auto GetMatrix() const->Matrix4x4f const&;
	auto GetRigidBodyMatrixInverse() const->Matrix4x4f;

protected:
	std::unique_ptr<SceneNode> _sceneNode = std::make_unique<SceneNode>();
	Vector3f _forwardDirection = Vector3f{ 0.0f, 1.0f, 0.0f };
	Vector3f _upwardDirection = Vector3f{ 0.0f, 0.0f, 1.0f };
	Vector3f _rightwardDirection = Vector3f{ 1.0f, 0.0f, 0.0f };
};

}