#include "ShaderProgram.h"

#include <GL/glew.h>

#include "Texture.h"

using std::string;

namespace core {

auto swap(ShaderProgram & first, ShaderProgram & second) -> void {
    using std::swap;
    swap(first._program, second._program);
    swap(first._shaders, second._shaders);
}

ShaderProgram::ShaderProgram(string const& vertexShaderFile, string const& fragmentShaderFile)
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
    assert(!_shaders.empty());
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
        MessageLogger::Log(MessageLogger::Error, "Shader linking failed: " + string{ log });
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

auto ShaderProgram::SetVertexShader(string const& filename) -> void {
    _shaders.push_back(Shader{ GL_VERTEX_SHADER, filename });
}

auto ShaderProgram::SetFragmentShader(string const& filename) -> void {
    _shaders.push_back(Shader{ GL_FRAGMENT_SHADER, filename });
}

auto ShaderProgram::SetTessellationControlShader(string const & filename) -> void {
    _shaders.push_back(Shader{ GL_TESS_CONTROL_SHADER, filename });
}

auto ShaderProgram::SetTessellationEvaluationShader(string const & filename) -> void {
    _shaders.push_back(Shader{ GL_TESS_EVALUATION_SHADER, filename });
}

auto ShaderProgram::Use() const -> void {
    assert(status::SentToCard == _status);
    glUseProgram(_program);

    // Do all the glUniform1i calls after loading the program, then never again.
    // You only need to call it once to tell the program which texture image unit each sampler uses.
    // After you've done that all you need to do is bind textures to the right texture image units.
    auto location = -1;
    if ((location = glGetUniformLocation(_program, "textures.diffuseMap")) != -1) {
        glUniform1i(location, TextureUsage::DiffuseMap);
    }
    if ((location = glGetUniformLocation(_program, "textures.specularMap")) != -1) {
        glUniform1i(location, TextureUsage::SpecularMap);
    }
    if ((location = glGetUniformLocation(_program, "textures.normalMap")) != -1) {
        glUniform1i(location, TextureUsage::NormalMap);
    }
    if ((location = glGetUniformLocation(_program, "textures.parallaxMap")) != -1) {
        glUniform1i(location, TextureUsage::ParallaxMap);
    }
    if ((location = glGetUniformLocation(_program, "textures.heightMap")) != -1) {
        glUniform1i(location, TextureUsage::HeightMap);
    }
    if ((location = glGetUniformLocation(_program, "textures.cubeMap")) != -1) {
        glUniform1i(location, TextureUsage::CubeMap);
    }
    if ((location = glGetUniformLocation(_program, "textures.diffuseTextureArray")) != -1) {
        glUniform1i(location, TextureUsage::DiffuseTextureArray);
    }
}

auto ShaderProgram::GetHandler() const -> GLuint {
    return _program;
}

}