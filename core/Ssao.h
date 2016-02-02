#pragma once

#include "Renderer.h"
#include "Primitive.h"
#include "Matrix.h"
#include "ShaderProgram.h"

namespace core {

class Ssao : public Renderer {
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
    ~Ssao() override;
public:
    auto Prepare() -> void override;
    auto DrawBegin() -> void override;
    auto DrawEnd() -> void override;
private:
    auto BindGBuffer() -> void;
    auto SsaoPass() -> void;
    auto LightingPass() -> void;
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