#pragma once

#include "Primitive.h"

namespace core {

template<typename T, size_type ROW, size_type COL>
class Matrix;

template<typename T>
struct Matrix_traits {
	using value_type = typename T::value_type;
};

template<typename T>
class MatrixExpression {
public:
	// use traits to avoid circular dependency
	using value_type = typename Matrix_traits<T>::value_type;

public:
	operator T&() {
		return static_cast<T&>(*this);
	}
	operator T const&() const {
		return static_cast<T const&>(*this);
	}
	auto operator()(size_type r, size_type c) const -> value_type {
		return static_cast<T const&>(*this)(r, c);
	}
	auto constexpr RowCount() const -> size_type {
		return static_cast<T const&>(*this).RowCount();
	}
	auto constexpr ColumnCount() const -> size_type {
		return static_cast<T const&>(*this).ColumnCount();
	}
};

template<typename T, size_type ROW, size_type COL>
class Matrix : public MatrixExpression<Matrix<T, ROW, COL>> {
public:
	using value_type = T;
public:
	Matrix() {
		_data.reserve(ROW * COL);
		for (auto i = 0u; i < ROW; ++i) {
			for (auto j = 0u; j < COL; ++j) {
				_data.push_back(i == j ? 1.0f : 0.0f);
			}
		}
	}
	Matrix(std::initializer_list<value_type> list) {
		_data = std::vector<value_type>{ list };
	}
	template<typename Expression>
	Matrix(Expression const& e) {
		_data.reserve(e.RowCount() * e.ColumnCount());
		for (auto i = 0u; i < e.RowCount(); ++i) {
			for (auto j = 0u; j < e.ColumnCount(); ++j) {
				_data.push_back(e(i, j));
			}
		}
	}

public:
	auto operator()(size_type r, size_type c) const -> value_type {
		assert(r < RowCount());
		assert(c < ColumnCount());
		return _data[r * ColumnCount() + c];
	}
	auto operator()(size_type r, size_type c) -> value_type& {
		assert(r < RowCount());
		assert(c < ColumnCount());
		return _data[r * ColumnCount() + c];
	}
	auto constexpr RowCount() const -> size_type {
		return ROW;
	}
	auto constexpr ColumnCount() const -> size_type {
		return COL;
	}
	auto data() -> value_type * {
		return _data.data();
	}
	auto data() const -> value_type const* {
		return _data.data();
	}

private:
	std::vector<value_type> _data;
};
using Matrix4x4f = Matrix<Float32, 4, 4>;

template<typename T, size_type ROW, size_type COL>
struct Matrix_traits<Matrix<T, ROW, COL>> {
	using value_type = T;
};

template<typename LHS, typename RHS>
class MatrixMultiply : public MatrixExpression<MatrixMultiply<LHS, RHS>> {
public:
	using value_type = typename LHS::value_type;

public:
	MatrixMultiply(MatrixExpression<LHS> const& lhs, MatrixExpression<RHS> const& rhs)
		: _lhs(lhs)
		, _rhs(rhs) {
		assert(lhs.ColumnCount() == rhs.RowCount());
	}

public:
	auto operator()(size_type r, size_type c) const -> value_type {
		assert(r < RowCount());
		assert(c < ColumnCount());
		auto ret = value_type(0);
		for (auto i = 0u; i < _lhs.ColumnCount(); ++i) {
			ret += _lhs(r, i) * _rhs(i, c);
		}
		return ret;
	}
	auto constexpr ColumnCount() const -> size_type {
		return _rhs.ColumnCount();
	}
	auto constexpr RowCount() const -> size_type {
		return _lhs.RowCount();
	}

private:
	MatrixExpression<LHS> const& _lhs;
	MatrixExpression<RHS> const& _rhs;
};

template<typename LHS, typename RHS>
struct Matrix_traits<MatrixMultiply<LHS, RHS>> {
	using value_type = typename LHS::value_type;
};

template<typename T1, typename T2>
auto operator*(MatrixExpression<T1> const& lhs, MatrixExpression<T2> const& rhs) {
	return MatrixMultiply<T1, T2>(lhs, rhs);
}

}
