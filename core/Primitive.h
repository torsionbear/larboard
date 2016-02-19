#pragma once

#include <vector>
#include <assert.h>
#include <cstdint>

namespace core {
// hack to avoid including glew.h in headers, which forces client code to indluce glew.h
using openglUint = unsigned int; // GLuint
using openglInt = int; // GLint
using openglSizei = int; // GLsizei
using openglEnum = unsigned int; // GLenum

using Float32 = float;
using size_type = unsigned int;
using uint8 = std::uint8_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;

static Float32 const pi = 3.1415926536f;

auto inline AlignedSize(unsigned int alignment, unsigned int actualSize) -> unsigned int {
	auto remainder = actualSize % alignment;
	return remainder == 0 ? actualSize : actualSize + alignment - remainder;
}

}