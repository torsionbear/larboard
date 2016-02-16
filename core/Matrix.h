#pragma once

#include <array>
#include "Primitive.h"

namespace core {

template<typename T>
struct Matrix_traits;

// Matrix expression
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
	auto operator()(size_type index) const -> value_type {
		return static_cast<T const&>(*this)(index);
	}
	auto constexpr RowCount() const -> size_type {
		return static_cast<T const&>(*this).RowCount();
	}
	auto constexpr ColumnCount() const -> size_type {
		return static_cast<T const&>(*this).ColumnCount();
	}
};

// Matrix
template<typename T, size_type ROW, size_type COL>
class Matrix : public MatrixExpression<Matrix<T, ROW, COL>> {
public:
	using value_type = T;
public:
	Matrix() {
		for (auto i = 0u; i < ROW; ++i) {
			for (auto j = 0u; j < COL; ++j) {
				_data[i * COL + j] = i == j ? 1.0f : 0.0f;
			}
		}
	}
	Matrix(std::initializer_list<value_type> list) {
		assert(list.size() == ROW * COL);
		auto i = 0u;
		for (auto v : list) {
			_data[i++] = v;
		}
	}
	template<typename T2>
	Matrix(MatrixExpression<T2> const& e) {
		assert(e.RowCount() == RowCount());
		assert(e.ColumnCount() == ColumnCount());
		for (auto i = 0u; i < e.RowCount(); ++i) {
			for (auto j = 0u; j < e.ColumnCount(); ++j) {
				_data[i * COL + j] = e(i, j);
			}
		}
	}

public:
	auto operator()(size_type r, size_type c) const -> value_type {
		assert(r < ROW);
		assert(c < COL);
		return _data[r * COL + c];
	}
	auto operator()(size_type r, size_type c) -> value_type& {
		assert(r < ROW);
		assert(c < COL);
		return _data[r * COL + c];
	}
	auto operator()(size_type index) const -> value_type {
		assert(index < ROW * COL);
		return _data[index];
	}
	auto operator()(size_type index) -> value_type & {
		assert(index < ROW * COL);
		return _data[index];
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
	std::array<value_type, ROW * COL> _data;
};
using Matrix3x4f = Matrix<Float32, 3, 4>;
using Matrix4x4f = Matrix<Float32, 4, 4>;
using Vector2f = Matrix<Float32, 2, 1>;
using Vector2i = Matrix<int, 2, 1>;
using Vector2i32 = Matrix<Int32, 2, 1>;
using Vector3f = Matrix<Float32, 3, 1>;
using Vector4f = Matrix<Float32, 4, 1>;
using Point4f = Matrix<Float32, 4, 1>;

template<typename T, size_type ROW, size_type COL>
struct Matrix_traits<Matrix<T, ROW, COL>> {
	using value_type = T;
};

template<typename T>
auto Length(Matrix<T, 4, 1> const& vector) -> T {
    return sqrt(vector(0) * vector(0) + vector(1) * vector(1) + vector(2) * vector(2));
}

template<typename T>
auto Normalize(Matrix<T, 4, 1> const& vector) -> Matrix<T, 4, 1> {
    auto lengthInverse = 1.0f / sqrt(vector(0) * vector(0) + vector(1) * vector(1) + vector(2) * vector(2));
    return Matrix<T, 4, 1>{ vector(0) * lengthInverse , vector(1) * lengthInverse , vector(2) * lengthInverse, 0 };
}

template<typename T>
auto Normalize(Matrix<T, 3, 1> const& vector) -> Matrix<T, 3, 1> {
    auto lengthInverse = 1.0f / sqrt(vector(0) * vector(0) + vector(1) * vector(1) + vector(2) * vector(2));
    return Matrix<T, 3, 1>{ vector(0) * lengthInverse, vector(1) * lengthInverse, vector(2) * lengthInverse};
}

template<typename T, size_type ROW>
auto DotProduct(Matrix<T, ROW, 1> const& lhs, Matrix<T, ROW, 1> const& rhs) -> T {
    auto ret = 0.0f;
    for (auto i = 0; i < ROW; ++i) {
        ret += lhs(i) * rhs(i);
    }
    return ret;
}

// Matrix product
template<typename LHS, typename RHS>
class MatrixProduct : public MatrixExpression<MatrixProduct<LHS, RHS>> {
public:
	using value_type = typename LHS::value_type;

public:
	MatrixProduct(MatrixExpression<LHS> const& lhs, MatrixExpression<RHS> const& rhs)
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
	auto operator()(size_type index) const -> value_type {
		auto r = index / ColumnCount();
		auto c = index % ColumnCount();
		return operator()(r, c);
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
struct Matrix_traits<MatrixProduct<LHS, RHS>> {
	using value_type = typename LHS::value_type;
};

template<typename LHS, typename RHS>
auto inline operator*(MatrixExpression<LHS> const& lhs, MatrixExpression<RHS> const& rhs) {
	return MatrixProduct<LHS, RHS>(lhs, rhs);
}

// Matrix Transpose
template <typename T>
class MatrixTranspose : public MatrixExpression<MatrixTranspose<T>> {
public:
	using value_type = typename T::value_type;
public:
	MatrixTranspose(MatrixExpression<T> const& matrix)
		: _matrix(matrix) {
	}
public:
	auto operator()(size_type r, size_type c) const -> value_type {
		return _matrix(c, r);
	}
	auto operator()(size_type index) const -> value_type {
		auto r = index / ColumnCount();
		auto c = index % ColumnCount();
		return _matrix(c, r);
	}
	auto constexpr ColumnCount() const {
		return _matrix.RowCount();
	}
	auto constexpr RowCount() const {
		return _matrix.ColumnCount();
	}
private:
	MatrixExpression<T> const& _matrix;
};

template <typename T>
struct Matrix_traits<MatrixTranspose<T>> {
	using value_type = typename T::value_type;
};

template <typename T>
auto inline Transpose(MatrixExpression<T> const& e) {
	return MatrixTranspose<T>(e);
}

// Matrix Negate
template <typename T>
class MatrixNegate : public MatrixExpression<MatrixNegate<T>> {
public:
	using value_type = typename T::value_type;
public:
	MatrixNegate(MatrixExpression<T> const& matrix)
		: _matrix(matrix) {
	}
public:
	auto operator()(size_type r, size_type c) const -> value_type {
		assert(r < RowCount());
		assert(c < ColumnCount());
		return -_matrix(r, c);
	}
	auto operator()(size_type index) const -> value_type {
		assert(index < RowCount() * ColumnCount());
		return -_matrix(index);
	}
	auto constexpr ColumnCount() const -> size_type {
		return _matrix.ColumnCount();
	}
	auto constexpr RowCount() const -> size_type {
		return _matrix.RowCount();
	}
private:
	MatrixExpression<T> const& _matrix;
};

template<typename T>
struct Matrix_traits<MatrixNegate<T>> {
	using value_type = typename T::value_type;
};

template<typename T>
auto inline operator-(MatrixExpression<T> const& expression) {
	return MatrixNegate<T>(expression);
}

// Matrix multiply scalar
template <typename T>
class MatrixScalarMultiply : public MatrixExpression<MatrixScalarMultiply<T>> {
public:
	using value_type = typename T::value_type;
public:
	MatrixScalarMultiply(MatrixExpression<T> const& matrix, value_type scalar)
		: _matrix(matrix)
		, _scalar(scalar) {
	}
public:
	auto operator()(size_type r, size_type c) const -> value_type {
		assert(r < RowCount());
		assert(c < ColumnCount());
		return _scalar * _matrix(r, c);
	}
	auto operator()(size_type index) const -> value_type {
		assert(index < RowCount() * ColumnCount());
		return _scalar * _matrix(index);
	}
	auto constexpr ColumnCount() const -> size_type {
		return _matrix.ColumnCount();
	}
	auto constexpr RowCount() const -> size_type {
		return _matrix.RowCount();
	}
private:
	MatrixExpression<T> const& _matrix;
	value_type _scalar;
};

template<typename T>
struct Matrix_traits<MatrixScalarMultiply<T>> {
	using value_type = typename T::value_type;
};

template<typename T>
auto inline operator*(MatrixExpression<T> const& expression, typename T::value_type scalar) {
	return MatrixScalarMultiply<T>(expression, scalar);
}

template<typename T>
auto inline operator/(MatrixExpression<T> const& expression, typename T::value_type scalar) {
    return MatrixScalarMultiply<T>(expression, 1/scalar);
}

template<typename T>
auto inline operator*(typename T::value_type scalar, MatrixExpression<T> const& expression) {
	return MatrixScalarMultiply<T>(expression, scalar);
}

// Matrix add
template <typename LHS, typename RHS> 
class MatrixAdd : public MatrixExpression<MatrixAdd<LHS, RHS>> {
public:
	using value_type = typename LHS::value_type;
public:
	MatrixAdd(MatrixExpression<LHS> const& lhs, MatrixExpression<RHS> const& rhs)
		: _lhs(lhs)
		, _rhs(rhs) {
	}
public:
	auto operator()(size_type r, size_type c) const -> value_type {
		assert(r < RowCount());
		assert(c < ColumnCount());
		return _lhs(r, c) + _rhs(r, c);
	}
	auto operator()(size_type index) const -> value_type {
		assert(index < RowCount() * ColumnCount());
		return _lhs(index) + _rhs(index);
	}
	auto constexpr RowCount() const -> size_type {
		return _lhs.RowCount();
	}
	auto constexpr ColumnCount() const -> size_type {
		return _rhs.ColumnCount();
	}
private:
	MatrixExpression<LHS> const& _lhs;
	MatrixExpression<RHS> const& _rhs;
};

template <typename LHS, typename RHS>
struct Matrix_traits<MatrixAdd<LHS, RHS>> {
	using value_type = typename LHS::value_type;
};

template<typename LHS, typename RHS>
auto inline operator+(MatrixExpression<LHS> const& lhs, MatrixExpression<RHS> const& rhs) {
	return MatrixAdd<LHS, RHS>(lhs, rhs);
}

// Matrix subtract
template <typename LHS, typename RHS>
class MatrixSubtract : public MatrixExpression<MatrixSubtract<LHS, RHS>> {
public:
    using value_type = typename LHS::value_type;
public:
    MatrixSubtract(MatrixExpression<LHS> const& lhs, MatrixExpression<RHS> const& rhs)
        : _lhs(lhs)
        , _rhs(rhs) {
    }
public:
    auto operator()(size_type r, size_type c) const -> value_type {
        assert(r < RowCount());
        assert(c < ColumnCount());
        return _lhs(r, c) - _rhs(r, c);
    }
    auto operator()(size_type index) const -> value_type {
        assert(index < RowCount() * ColumnCount());
        return _lhs(index) - _rhs(index);
    }
    auto constexpr RowCount() const -> size_type {
        return _lhs.RowCount();
    }
    auto constexpr ColumnCount() const -> size_type {
        return _rhs.ColumnCount();
    }
private:
    MatrixExpression<LHS> const& _lhs;
    MatrixExpression<RHS> const& _rhs;
};

template <typename LHS, typename RHS>
struct Matrix_traits<MatrixSubtract<LHS, RHS>> {
    using value_type = typename LHS::value_type;
};

template<typename LHS, typename RHS>
auto inline operator-(MatrixExpression<LHS> const& lhs, MatrixExpression<RHS> const& rhs) {
    return MatrixSubtract<LHS, RHS>(lhs, rhs);
}

// 3-dimention vector cross product
template<typename T>
class VectorCrossProduct : public MatrixExpression<VectorCrossProduct<T>> {
public:
	using value_type = typename T::value_type;
public:
	VectorCrossProduct(MatrixExpression<T> const& lhs, MatrixExpression<T> const& rhs)
		: _lhs(lhs)
		, _rhs(rhs) {
		assert(_lhs.ColumnCount() == 1);
		assert(_lhs.RowCount() == 3 || _lhs.RowCount() == 4);
		assert(_rhs.ColumnCount() == 1);
		assert(_rhs.RowCount() == 3 || _rhs.RowCount() == 4);
	}
public:
	auto operator()(size_type r, size_type c) const -> value_type {
		assert(c < ColumnCount());
		assert(r < RowCount());
		switch (r) {
		case 0:
			return _lhs(1) * _rhs(2) - _lhs(2) * _rhs(1);
		case 1:
			return _lhs(2) * _rhs(0) - _lhs(0) * _rhs(2);
		case 2:
			return _lhs(0) * _rhs(1) - _lhs(1) * _rhs(0);
		case 3:
			return 0;
		}
		return 0;
	}
	auto operator()(size_type index) const -> value_type {
		auto r = index / ColumnCount();
		auto c = index % ColumnCount();
		return operator()(r, c)
	}
	auto constexpr RowCount() const -> size_type {
		return _lhs.RowCount();
	}
	auto constexpr ColumnCount() const -> size_type {
		return _lhs.ColumnCount();
	}
private:
	MatrixExpression<T> const& _lhs;
	MatrixExpression<T> const& _rhs;
};

template <typename T>
struct Matrix_traits<VectorCrossProduct<T>> {
	using value_type = typename T::value_type;
};

template<typename T>
auto inline CrossProduct(MatrixExpression<T> const& lhs, MatrixExpression<T> const& rhs) {
	return VectorCrossProduct<T>(lhs, rhs);
}


}
