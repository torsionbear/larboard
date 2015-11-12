#include "ShaderProgram.h"

#include <GL/glew.h>

#include "Texture.h"

namespace core {

auto swap(ShaderProgram & first, ShaderProgram & second) -> void {
	using std::swap;
	swap(first._program, second._program);
	swap(first._shaders, second._shaders);
}

ShaderProgram::ShaderProgram(std::string const& vertexShaderFile, std::string const& fragmentShaderFile)
	: _program(0)
	, _shaders{ Shader{ GL_VERTEX_SHADER, vertexShaderFile }, Shader{ GL_FRAGMENT_SHADER, fragmentShaderFile } } {
}

ShaderProgram::ShaderProgram(ShaderProgram && other) {
	swap(*this, other);
}

auto ShaderProgram::operator=(ShaderProgram && other) -> ShaderProgram & {
	swap(*this, other);
	return *this;
}

ShaderProgram::~ShaderProgram() = default;

auto ShaderProgram::LoadImpl() -> bool {
	for (auto& shader : _shaders) {
		if (!shader.Load()) {
			return false;
		}
	}
	return true;
}
auto ShaderProgram::UnloadImpl() -> bool {
	for (auto& shader : _shaders) {
		shader.Unload();
	}
	return true;
}
auto ShaderProgram::SendToCardImpl() -> bool {
	_program = glCreateProgram();

	for (auto& shader : _shaders) {
		glAttachShader(_program, shader.Compile());
	}
	glLinkProgram(_program);

	GLint linked;
	glGetProgramiv(_program, GL_LINK_STATUS, &linked);
	if (!linked) {
		GLsizei len;
		glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &len);

		GLchar* log = new GLchar[len + 1];
		glGetProgramInfoLog(_program, len, &len, log);
		MessageLogger::Log(MessageLogger::Error, "Shader linking failed: " + std::string{ log });
		delete[] log;
		_shaders.clear();
		return false;
	}
	for (auto& shader : _shaders) {
		shader.DeleteShader();
	}

	// no longer bind uniform block with uniform buffer index here,
	// instead we specify binding in shader's interface block layout.
	//glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "Camera"), GetIndex(UniformBufferType::Viewpoint));
	//glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "Transform"), GetIndex(UniformBufferType::Transform));
	//glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "Material"), GetIndex(UniformBufferType::Material));

	return true;
}
auto ShaderProgram::FreeFromCardImpl() -> bool {
	glDeleteProgram(_program);
	return true;
}

auto ShaderProgram::SetVertexShader(std::string const& filename) -> void {
	_shaders[0] = Shader{ GL_VERTEX_SHADER, filename };
}

auto ShaderProgram::SetFragmentShader(std::string const& filename) -> void {
	_shaders[1] = Shader{ GL_VERTEX_SHADER, filename };
}

auto ShaderProgram::AddShader(std::string const& filename) -> void {
	throw "not implemented yet";
}

auto ShaderProgram::Use() const -> void {
	assert(status::SentToCard == _status);

	glUseProgram(_program);

	// Do all the glUniform1i calls after loading the program, then never again.
	// You only need to call it once to tell the program which texture image unit each sampler uses.
	// After you've done that all you need to do is bind textures to the right texture image units.
	auto location = -1;
	if (location = glGetUniformLocation(_program, "textures.diffuseMap") != -1) {
		glUniform1i(location, Texture::DiffuseMap);
	}
	if (location = glGetUniformLocation(_program, "textures.specularMap") != -1) {
		glUniform1i(location, Texture::SpecularMap);
	}
	if (location = glGetUniformLocation(_program, "textures.normalMap") != -1) {
		glUniform1i(location, Texture::NormalMap);
	}
	if (location = glGetUniformLocation(_program, "textures.parallaxMap") != -1) {
		glUniform1i(location, Texture::ParallaxMap);
	}
    if (location = glGetUniformLocation(_program, "textures.heightMap") != -1) {
        glUniform1i(location, Texture::HeightMap);
    }
    auto error = glGetError();
}

auto ShaderProgram::GetHandler() -> GLuint {
	return _program;
}

}