#pragma once

#include <array>

#include "Primitive.h"

namespace core {

template<typename T>
struct Vector_traits;

template <typename T>
class VectorExpression {
public:
	using value_type = typename Vector_traits<T>::value_type;
public:
	operator T & () {
		return static_cast<T &>(*this);
	}
	operator T const& () const {
		return static_cast<T const&>(*this);
	}
	auto operator()(size_type index) const -> value_type {
		return static_cast<T const&>(*this)(index);
	}
	auto constexpr length() const -> size_type {
		return static_cast<T const&>(*this).length();
	}
};

template <typename T, size_type LENGTH>
class Vector : public VectorExpression<Vector<T, LENGTH>> {
public:
	using value_type = T;
public:
	Vector() {
	}
	Vector(std::initializer_list<value_type> const& values) {
		// haven't found a way to directly initialize std::array...
		assert(values.size() <= _data.size());
		auto i = 0u;
		for(auto v : values) {
			_data[i++] = v;
		}
	}
	template <typename EXPRESSION>
	Vector(EXPRESSION const& expression) {
		for (auto i = 0u; i < LENGTH; ++i) {
			_data[i] = expression(i);
		}
	}
public:
	auto operator()(size_type index) const -> value_type const& {
		assert(index < LENGTH);
		return _data[index];
	}
	auto operator()(size_type index) -> value_type & {
		assert(index < LENGTH);
		return _data[index];
	}
	auto constexpr length() const -> size_type {
		return LENGTH;
	}
private:
	// use std::array (rather than std::vector) so data is stored inside object rather than on heap.
	// By doing this, std::vector<Vector<Float32, 3>>::data() can be used as input to 
	// GL functions which requires void* pointing to data.
	std::array<T, LENGTH> _data = {}; // default initialize std::array
};
using Vector2f = Vector<Float32, 2>;
using Vector3f = Vector<Float32, 3>;
using Vector4f = Vector<Float32, 4>;

template <typename T, size_type LENGTH>
struct Vector_traits<Vector<T, LENGTH>> {
	using value_type = T;
};

template<typename T1, typename T2>
class VectorAdd : public VectorExpression<VectorAdd<T1, T2>> {
public:
	using value_type = typename T1::value_type;
public:
	VectorAdd(VectorExpression<T1> const& lhs, VectorExpression<T2> const& rhs)
		: _lhs(lhs)
		, _rhs(rhs) {
		assert(lhs.length() == rhs.length());
	}
public:
	auto operator()(size_type index) const -> value_type {
		assert(index < length());
		return _lhs(index) + _rhs(index);
	}
	auto constexpr length() const -> size_type {
		return _lhs.length();
	}
private:
	VectorExpression<T1> const& _lhs;
	VectorExpression<T2> const& _rhs;
};

template <typename T1, typename T2>
struct Vector_traits<VectorAdd<T1, T2>> {
	using value_type = typename T1::value_type;
};

template<typename T1, typename T2>
auto inline operator+(VectorExpression<T1> const& lhs, VectorExpression<T2> const& rhs) {
	return VectorAdd<T1, T2>(lhs, rhs);
}

template<typename T>
class VectorNegate : public VectorExpression<VectorNegate<T>> {
public:
	using value_type = typename T::value_type;
public:
	VectorNegate(T const& v)
		: _vector(v) {
	}
public:
	auto operator()(size_type index) const -> value_type {
		assert(index < length());
		return -_vector(index);
	}
	auto constexpr length() const -> size_type {
		return _vector.length();
	}
private:
	T const& _vector;
};

template <typename T>
struct Vector_traits<VectorNegate<T>> {
	using value_type = typename T::value_type;
};

template<typename T>
auto inline operator-(T const& v) {
	return VectorNegate<T>(v);
} 


template<typename T>
class VectorCrossProduct : public VectorExpression<VectorCrossProduct<T>> {
public:
	using value_type = typename T::value_type;
public:
	VectorCrossProduct(T const& lhs, T const& rhs)
		: _lhs(lhs)
		, _rhs(rhs) {
		assert(lhs.length() == 3 && rhs.length() == 3);	// only support 3 dimensional vector cross product
	}
public:
	auto operator()(size_type index) const -> value_type {
		assert(index < length());
		switch (index) {
		case 0:
			return _lhs(1) * _rhs(2) - _lhs(2) * _rhs(1);
		case 1:
			return _lhs(2) * _rhs(0) - _lhs(0) * _rhs(2);
		case 2:
			return _lhs(0) * _rhs(1) - _lhs(1) * _rhs(0);
		}
		return 0;
	}
	auto constexpr length() const -> size_type {
		return _lhs.length();
	}
private:
	T const& _lhs;
	T const& _rhs;
};

template <typename T>
struct Vector_traits<VectorCrossProduct<T>> {
	using value_type = typename T::value_type;
};

template<typename T>
auto inline operator*(VectorExpression<T> const& lhs, VectorExpression<T> const& rhs) {
	return VectorCrossProduct<VectorExpression<T>>(lhs, rhs);
}

}