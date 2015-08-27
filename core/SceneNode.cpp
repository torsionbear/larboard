#include "SceneNode.h"
#include <math.h>

using std::move;
using std::unique_ptr;

namespace core {

SceneNode::SceneNode() = default;
SceneNode::~SceneNode() = default;

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

auto SceneNode::Rotate(Float32 x, Float32 y, Float32 z, Float32 r) -> void {
	auto a = cos(r);
	auto b = sin(r);
	auto c = 1 - a;
	_transform = Matrix4x4f{
		a + c*x*x,		c*x*y - b*z,	c*x*z + b*y,	0,
		c*x*y + b*z,	a + c*y*y,		c*y*z - b*x,	0,
		c*x*z - b*y,	c*y*z + b*x,	a + c*z*z,		0,
		0,				0,				0,				1
	} * _transform;
	for (auto c : _children) {
		c->Rotate(x, y, z, r);
	}
}

}
