#pragma once

#include <vector>
#include <memory>
#include <forward_list>

#include "Primitive.h"

namespace core {

class SceneNode {
public:
	friend class Movable;

public:
	SceneNode();
	SceneNode(SceneNode const&) = delete;
	SceneNode& operator=(SceneNode const&) = delete;
	~SceneNode();

private:
	auto MoveAlong(Vector3f const& forwardDirection, Float32 length) -> void;
	auto Translate(Float32, Float32, Float32) -> void;
	auto Rotate(Float32, Float32, Float32, Float32) -> void;

private:
	SceneNode* _parent = nullptr;
	std::forward_list<SceneNode*> _children;
	Matrix4x4f _transform;
};

}