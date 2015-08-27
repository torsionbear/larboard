#pragma once

#include "gtest/gtest.h"

#include "core/Endian.h"

class EndianTest : public ::testing::Test {

};

TEST_F(EndianTest, Convert_endianness) {
	auto uchar = 'a';
	ASSERT_EQ(uchar, Endian::ConvertBigEndian(uchar));
	ASSERT_EQ(uchar, Endian::ConvertLittleEndian(uchar));

	auto sshort = static_cast<short>(0xfeff);
	ASSERT_EQ(-2, Endian::ConvertBigEndian(sshort));
	ASSERT_EQ(sshort, Endian::ConvertLittleEndian(sshort));

	auto ushort = static_cast<unsigned short>(0x68a3);
	ASSERT_EQ(41832u, Endian::ConvertBigEndian(ushort));
	ASSERT_EQ(ushort, Endian::ConvertLittleEndian(ushort));

	auto uint = 0x7a3f68a3u;
	ASSERT_EQ(2741518202u, Endian::ConvertBigEndian(uint));
	ASSERT_EQ(uint, Endian::ConvertLittleEndian(uint));
}