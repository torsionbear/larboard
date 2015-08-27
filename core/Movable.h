#pragma once

#include <memory>

#include "SceneNode.h"

namespace core {

class Movable {
public:
	friend class Scene;

public:
	Movable() = default;
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
	auto Translate(Float32, Float32, Float32) -> void;
	auto Rotate(Float32, Float32, Float32, Float32) -> void;
	auto AttachTo(Movable &) -> void;
	auto DetachFrom() -> void;
	auto GetMatrix() const -> Matrix4x4f const&;
	auto GetRigidBodyMatrixInverse() const -> Matrix4x4f;

protected:
	std::unique_ptr<SceneNode> _sceneNode = std::make_unique<SceneNode>();
};

}