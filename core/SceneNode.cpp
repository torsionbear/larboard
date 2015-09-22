#include "SceneNode.h"
#include <math.h>

using std::move;
using std::unique_ptr;

namespace core {

SceneNode::SceneNode() = default;
SceneNode::~SceneNode() = default;

auto SceneNode::MoveAlong(Vector3f const& forwardDirection, Float32 length) -> void {
	_transform = _transform * Matrix4x4f{
		1, 0, 0, forwardDirection(0) * length,
		0, 1, 0, forwardDirection(1) * length,
		0, 0, 1, forwardDirection(2) * length,
		0, 0, 0, 1
	};
}

auto SceneNode::Translate(Float32 x, Float32 y, Float32 z) -> void {
	_transform = Matrix4x4f{
		1, 0, 0, x,
		0, 1, 0, y,
		0, 0, 1, z,
		0, 0, 0, 1
	} *_transform;
	for (auto c : _children) {
		c->Translate(x, y, z);
	}
}

auto SceneNode::Rotate(Float32 x, Float32 y, Float32 z, Float32 r, bool rotateSelf) -> void {
	auto a = cos(r);
	auto b = sin(r);
	auto c = 1 - a;
	auto rotateMatrix = Matrix4x4f{
		a + c*x*x,		c*x*y - b*z,	c*x*z + b*y,	0,
		c*x*y + b*z,	a + c*y*y,		c*y*z - b*x,	0,
		c*x*z - b*y,	c*y*z + b*x,	a + c*z*z,		0,
		0,				0,				0,				1
	};
	_transform = rotateSelf ? _transform * rotateMatrix : rotateMatrix *_transform;
	for (auto c : _children) {
		c->Rotate(x, y, z, r, rotateSelf);
	}
}

}
