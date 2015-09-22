#include "gtest/gtest.h"

#include "core/Vector.h"

using namespace core;

class VectorTest : public ::testing::Test {
public:
	template<typename T>
	auto equal(T v1, T v2) -> bool;

	template<>
	auto equal(Float32 v1, Float32 v2) -> bool {
		return abs(v1 - v2) < std::numeric_limits<Float32>::epsilon();
	}

	template <typename T, size_type LENGTH>
	auto equal(Vector<T, LENGTH> const& v1, Vector<T, LENGTH> const& v2) -> bool {
		for (auto i = 0u; i < v1.length(); ++i) {
			if (!equal(v1(i), v2(i))) {
				return false;
			}
		}
		return true;
	}
};

TEST_F(VectorTest, Vector_negate) {
	auto result = static_cast<Vector3f>(-Vector3f{ 1.0f, 2.0f, 3.0f });
	ASSERT_TRUE(equal(Vector3f{ -1.0f, -2.0f, -3.0f }, result));
}

TEST_F(VectorTest, Vector_add) {
	auto result = static_cast<Vector3f>(Vector3f{ 1.0f, 2.0f, 3.0f } + Vector3f{ -1.0f, -2.0f, -3.0f });
	ASSERT_TRUE(equal(Vector3f{ 0.0f, 0.0f, 0.0f }, result));	
}

TEST_F(VectorTest, Vector_cross_product) {
	auto result = static_cast<Vector3f>(Vector3f{ 0, 1, 0 } *Vector3f{ 0, 0, 1 });
	ASSERT_TRUE(equal(Vector3f{1.0f, 0.0f, 0.0f}, result));
}