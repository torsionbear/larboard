#include "core/PngReader.h"

#include "gtest/gtest.h"

using namespace core;

class PngReaderTest : public ::testing::Test {
public:
protected:
	//PngReader _sut{ "D:\\torsionbear\\working\\larboard\\Modeling\\square\\Pedobear.png" };
	PngReader _sut{ "D:\\torsionbear\\working\\larboard\\Modeling\\rgb.png" };
};

TEST_F(PngReaderTest, tmpCase) {
	_sut.ReadPng();
	ASSERT_EQ(10, _sut.Height());
	ASSERT_EQ(20, _sut.Width());

	auto data = _sut.GetData();
	for (auto i = 0u; i < 10; ++i) {
		for (auto j = 0u; j < 20; ++j) {
			auto pixelPos = i * 20 + j;
			switch (j % 3) {
			case 0:	// red				
				ASSERT_FLOAT_EQ(1, data[pixelPos](0));
				ASSERT_FLOAT_EQ(0, data[pixelPos](1));
				ASSERT_FLOAT_EQ(0, data[pixelPos](2));
				ASSERT_FLOAT_EQ(1, data[pixelPos](3));
				break;
			case 1:	// green
				ASSERT_FLOAT_EQ(0, data[pixelPos](0));
				ASSERT_FLOAT_EQ(1, data[pixelPos](1));
				ASSERT_FLOAT_EQ(0, data[pixelPos](2));
				ASSERT_FLOAT_EQ(1, data[pixelPos](3));
				break;
			case 2: // blue
				ASSERT_FLOAT_EQ(0, data[pixelPos](0));
				ASSERT_FLOAT_EQ(0, data[pixelPos](1));
				ASSERT_FLOAT_EQ(1, data[pixelPos](2));
				ASSERT_FLOAT_EQ(1, data[pixelPos](3));
				break;
			}
		}
	}
}