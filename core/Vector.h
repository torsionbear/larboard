#pragma once

#include "Primitive.h"

namespace core {

template <typename T>
struct Vector4 {
	T x;
	T y;
	T z;
	T w;
};

template <typename T>
struct Vector3 {
	Vector3<T> operator*(Vector3 const& rhs) {
		return Vector3<T>{y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x};
	}
	T x;
	T y;
	T z;
};
using Vector3f = Vector3<Float32>;
using Vector4f = Vector4<Float32>;

template <typename T>
struct Vector2 {
	T x;
	T y;
};
using Vector2f = Vector2<Float32>;

}