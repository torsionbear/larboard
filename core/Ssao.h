#pragma once

#include "Primitive.h"
#include "Matrix.h"
#include "ShaderProgram.h"

namespace core {

class Ssao {
    enum TextureBindingPoint : int {
        diffuseEmissive = 0,
        specularShininess = 1,
        normal = 2,
        occlusion = 3,
        randomVector = 4,
        depth = 9,
    };
public:
    Ssao(unsigned int width, unsigned int height);
    ~Ssao();
public:
    auto PrepareForDraw() -> void;
    auto BindGBuffer() -> void;
    auto SsaoPass() -> void;
    auto LightingPass() -> void;
private:
    auto InitSsaoFbo() -> void;
    auto GenerateSamples(unsigned int sampleCount, Vector2f sampleDistanceRange) -> void;
    auto GenerateRandomTexture(unsigned int textureSize) -> void;
private:
    unsigned int const _screenWidth;
    unsigned int const _screenHeight;
    openglUint _ssaoFbo;
    openglUint _diffuseEmissiveBuffer;
    openglUint _specularShininessBuffer;
    openglUint _ssaoNormalBuffer;
    openglUint _ssaoOcclusionBuffer;
    openglUint _ssaoDepthBuffer;
    ShaderProgram _ssaoShaderProgram;
    ShaderProgram _lightingShaderProgram;

    openglUint _screenQuadVao;
    openglUint _screenQuadVbo;
    openglUint _randomVectorTexture;
};

}