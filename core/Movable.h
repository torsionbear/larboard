#pragma once

#include <memory>

#include "Matrix.h"
#include "SceneNode.h"

namespace core {

class Movable {
public:
	friend class Scene;
public:
	struct ShaderData {
    public:
		Matrix4x4f worldTransform;
		Matrix4x4f normalTransform;
    public:
        static auto Size() -> unsigned int;
	};

public:
	Movable() = default;
	Movable(Vector4f const& forwardDirection, Vector4f const& upwardDirection)
		: _forwardDirection(forwardDirection) 
		, _upwardDirection(upwardDirection) 
		, _rightDirection(CrossProduct(_forwardDirection, _upwardDirection)) {		
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
	auto Head(Float32 radius) -> void;
	auto Pitch(Float32 radius) -> void;
	auto Roll(Float32 radius) -> void;

	auto Translate(Float32, Float32, Float32) -> void;
    auto Translate(Vector4f const&) -> void;
	auto Rotate(Float32, Float32, Float32, Float32) -> void;
	auto Rotate(Vector4f const& pivot, Float32 angle) -> void;
	auto Rotate(Point4f const& center, Vector4f const& pivot, Float32 angle) -> void;

	auto GetRightDirection() -> Vector4f;
	auto GetForwardDirection() -> Vector4f;
	auto GetUpwardDirection() -> Vector4f;

	auto AttachTo(Movable &) -> void;
	auto DetachFrom() -> void;

	auto GetPosition() const -> Point4f;
    auto GetRotationInverse() const -> Matrix4x4f;
	auto GetTransform() const -> Matrix4x4f const&;
	auto GetRigidBodyMatrixInverse() const->Matrix4x4f;
	auto GetNormalTransform() const -> Matrix4x4f;

	auto GetUboOffset() const -> int {
		return _uboOffset;
	}
	auto SetUboOffset(int offset) -> void {
		_uboOffset = offset;
	}
	auto GetShaderData() const -> ShaderData;

protected:
	std::unique_ptr<SceneNode> _sceneNode = std::make_unique<SceneNode>();
	Vector4f _forwardDirection = Vector4f{ 0.0f, 1.0f, 0.0f, 0.0f };
	Vector4f _upwardDirection = Vector4f{ 0.0f, 0.0f, 1.0f, 0.0f };
	Vector4f _rightDirection = Vector4f{ 1.0f, 0.0f, 0.0f, 0.0f };

	openglInt _uboOffset;
};

}