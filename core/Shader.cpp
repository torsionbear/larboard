#include "Shader.h"

#include <fstream>

#include "gl/glew.h"

#include "MessageLogger.h"

namespace core {


auto Shader::Load() -> bool {
    std::ifstream file{ _filename };
    if (!file) {
        MessageLogger::Log(MessageLogger::Error, "Unable to open file.");
        return false;
    }
    _source = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    return true;
}
auto Shader::Unload() -> void {
    _source.clear();
}
auto Shader::Compile() -> GLuint {
    _shader = glCreateShader(_type);
    char const* src = _source.c_str();
    glShaderSource(_shader, 1, &src, nullptr);
    glCompileShader(_shader);

    GLint compiled;
    glGetShaderiv(_shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLsizei len;
        glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &len);

        GLchar* log = new GLchar[len + 1];
        glGetShaderInfoLog(_shader, len, &len, log);
        MessageLogger::Log(MessageLogger::Error, "Shader compilation failed: " + std::string{ log });
        delete[] log;
    }
    return _shader;
}
auto Shader::DeleteShader() -> void {
    glDeleteShader(_shader);
}

}