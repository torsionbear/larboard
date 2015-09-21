#pragma once

#include <vector>
#include <assert.h>

namespace core {
// hack to avoid including glew.h in headers, which forces client code to indluce glew.h
using openglUint = unsigned int; // GLuint
using openglInt = int; // GLint
using openglSizei = int; // GLsizei
using openglEnum = unsigned int; // GLenum

using Float32 = float;
using size_type = unsigned int;

static Float32 const pi = 3.1415926536f;

}