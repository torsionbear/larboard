#pragma once

#include "Primitive.h"

namespace core {

class Shader {
public:
    enum Type {
        Vertex,
        Fragment,
        Geometry,
        TessellationControl,
        TessellationEvaluation,
    };
public:
    Shader(openglEnum type, std::string const& filename)
        : _type(type)
        , _filename(filename) {
    }

public:
    auto Load() -> bool;
    auto Unload() -> void;
    auto Compile()->openglUint;
    auto DeleteShader() -> void;

private:
    std::string _filename;
    std::string _source;
    openglEnum _type;
    openglUint _shader;
};

}