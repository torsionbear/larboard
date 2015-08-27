#include "gtest/gtest.h"

#include "core/primitive.h"

using namespace core;

class MatrixTest : public ::testing::Test {

};

template <typename T>
auto equal(T v1, T v2) -> bool;

template <>
auto equal(Float32 v1, Float32 v2) -> bool {
	return abs(v1 - v2) < std::numeric_limits<Float32>::epsilon();
}

template<typename T, size_type ROW, size_type COL>
auto equal(const Matrix<T, ROW, COL>& lhs, const Matrix<T, ROW, COL>& rhs) -> bool {
	if (lhs.RowCount() != rhs.RowCount() || lhs.ColumnCount() != rhs.ColumnCount()) {
		return false;
	}
	for (auto i = 0u; i < lhs.RowCount(); ++i) {
		for (auto j = 0u; i < lhs.ColumnCount(); ++i) {
			if (!equal(lhs(i, j), rhs(i, j))) {
				return false;
			}
		}
	}
	return true;
}

TEST_F(MatrixTest, Matrix_multiplification) {
	auto identity = Matrix4x4f{};
	auto ones = Matrix4x4f{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };

	auto i = static_cast<Matrix4x4f>(ones*identity);
	ASSERT_TRUE(equal(ones, i));

	auto threes = static_cast<Matrix4x4f>(ones*ones);
	assert(equal(threes, Matrix4x4f{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, }));

	auto m = Matrix4x4f{ 1, 2, 3, 4, 4, 3, 2, 1, 3, 4, 1, 2, 2, 1, 4, 3 };
	auto antiDiagonal = Matrix4x4f{ 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, };
	auto result = static_cast<Matrix4x4f>(m*antiDiagonal*identity);
	ASSERT_TRUE(equal(result, Matrix4x4f{ 4, 3, 2, 1, 1, 2, 3, 4, 2, 1, 4, 3, 3, 4, 1, 2 }));
}