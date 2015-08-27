#pragma once

#include "Primitive.h"

namespace core {

class Shader {
public:
	Shader(openglEnum type, std::string const& filename)
		: m_Type(type)
		, _filename(filename) {
	}

public:
	auto Load() -> bool;
	auto Unload() -> void;
	auto Compile() -> openglUint;
	auto DeleteShader() -> void;

private:
	std::string _filename;
	std::string _source;
	openglEnum m_Type;
	openglUint _shader;
};

}