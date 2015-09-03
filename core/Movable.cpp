#include "Movable.h"

#include <assert.h>

namespace core {

auto swap(Movable & first, Movable & second) -> void {
	swap(first._sceneNode, second._sceneNode);
}

auto Movable::Forward(Float32 length) -> void {
	_sceneNode->MoveAlong(_forwardDirection, length);
}

auto Movable::Backward(Float32 length) -> void {
	_sceneNode->MoveAlong(_forwardDirection, -length);
}

auto Movable::Leftward(Float32 length) -> void {
	_sceneNode->MoveAlong(_rightwardDirection, -length);
}

auto Movable::Rightward(Float32 length) -> void {
	_sceneNode->MoveAlong(_rightwardDirection, length);
}

auto Movable::Upward(Float32 length) -> void {
	_sceneNode->MoveAlong(_upwardDirection, length);
}

auto Movable::Downward(Float32 length) -> void {
	_sceneNode->MoveAlong(_upwardDirection, -length);
}

auto Movable::Translate(Float32 x, Float32 y, Float32 z) -> void {
	assert(nullptr != _sceneNode);
	_sceneNode->Translate(x, y, z);
}

auto Movable::Rotate(Float32 x, Float32 y, Float32 z, Float32 r) -> void {
	assert(nullptr != _sceneNode);
	_sceneNode->Rotate(x, y, z, r);
}

auto Movable::AttachTo(Movable & node) -> void {
	_sceneNode->_parent = node._sceneNode.get();
	node._sceneNode->_children.push_front(_sceneNode.get());
}
auto Movable::DetachFrom() -> void {
	_sceneNode->_parent->_children.remove(_sceneNode.get());
	_sceneNode->_parent = nullptr;
}

auto Movable::GetMatrix() const -> Matrix4x4f const& {
	return _sceneNode->_transform;
}

auto Movable::GetRigidBodyMatrixInverse() const -> Matrix4x4f {
	using std::swap;
	auto const& matrix = _sceneNode->_transform;

	// rigid body matrix inverse: 
	// 1. transpose rotation part (left-upper 3x3 submatrix)
	auto rotation = matrix;
	rotation(0, 3) = rotation(1, 3) = rotation(2, 3) = 0;
	swap(rotation(0, 1), rotation(1, 0));
	swap(rotation(0, 2), rotation(2, 0));
	swap(rotation(1, 2), rotation(2, 1));
	// 2. negate translation part (first 3 elements of 4th column)
	auto translation = Matrix4x4f{
		1, 0, 0, -matrix(0, 3),
		0, 1, 0, -matrix(1, 3),
		0, 0, 1, -matrix(2, 3),
		0, 0, 0, 1,
	};
	return rotation * translation;
}

}