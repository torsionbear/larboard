#pragma once

#include "Primitive.h"
#include "ShaderProgram.h"

namespace core {

class Ssao {
public:
    Ssao(unsigned int width, unsigned int height);
    ~Ssao();
public:
    auto PrepareForDraw() -> void;
    auto BindGBuffer() -> void;
    auto UnbindGBuffer() -> void;
    auto DeferredPass() -> void;
private:
    auto InitFbo() -> void;
private:
    unsigned int const _screenWidth;
    unsigned int const _screenHeight;
    openglUint _gBuffer;
    openglUint _fboColorBuffer;
    openglUint _fboNormalBuffer;
    openglUint _fboDepthBuffer;
    openglUint _deferredPassVao;
    openglUint _deferredPassVbo;
    ShaderProgram _deferredPassShaderProgram;
};

}