#pragma once

#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
#include <assert.h>

#include "Shader.h"
#include "MessageLogger.h"
#include "Resource.h"
#include "Primitive.h"

namespace core {

class ShaderProgram : public Resource {
public:
	friend auto swap(ShaderProgram & first, ShaderProgram & second) -> void;
	ShaderProgram(std::string const& vertexShaderFile, std::string const& fragmentShaderFile);
	ShaderProgram(ShaderProgram const &) = delete;
	ShaderProgram(ShaderProgram && other);
	ShaderProgram& operator=(ShaderProgram const&) = delete;
	ShaderProgram& operator=(ShaderProgram && other);
	~ShaderProgram();

private:
	auto LoadImpl() -> bool override;
	auto UnloadImpl() -> bool override;
	auto SendToCardImpl() -> bool override;
	auto FreeFromCardImpl() -> bool override;

public:
	auto SetVertexShader(std::string const& filename) -> void;
	auto SetFragmentShader(std::string const& filename) -> void;
	auto AddShader(std::string const& filename) -> void;
	auto Use() -> void;
	auto GetHandler() -> openglUint;
	
private:
	openglUint _program = 0;
	std::vector<Shader> _shaders;
};

}