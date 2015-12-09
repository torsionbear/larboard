#include "Ssao.h"

#include <array>
#include <vector>
#include <random>

#include <GL/glew.h>

using std::array;
using std::vector;

namespace core {

Ssao::Ssao(unsigned int width, unsigned int height)
    : _screenWidth(width)
    , _screenHeight(height)
    , _ssaoShaderProgram("shader/ssao_v.shader", "shader/ssao_f.shader")
    , _lightingShaderProgram("shader/ssaoLighting_v.shader", "shader/ssaoLighting_f.shader") {
}

Ssao::~Ssao() {
    glDeleteBuffers(1, &_screenQuadVao);
    glDeleteBuffers(1, &_screenQuadVbo);
    glDeleteFramebuffers(1, &_ssaoFbo);
}

auto Ssao::PrepareForDraw() -> void {
    InitSsaoFbo();
    _ssaoShaderProgram.SendToCard();
    //glUseProgram(_ssaoShaderProgram.GetHandler());
    glProgramUniform1i(_ssaoShaderProgram.GetHandler(), glGetUniformLocation(_ssaoShaderProgram.GetHandler(), "gBuffer.diffuseEmissive"), TextureBindingPoint::diffuseEmissive);
    glProgramUniform1i(_ssaoShaderProgram.GetHandler(), glGetUniformLocation(_ssaoShaderProgram.GetHandler(), "gBuffer.specularShininess"), TextureBindingPoint::specularShininess);
    glProgramUniform1i(_ssaoShaderProgram.GetHandler(), glGetUniformLocation(_ssaoShaderProgram.GetHandler(), "gBuffer.normal"), TextureBindingPoint::normal);
    glProgramUniform1i(_ssaoShaderProgram.GetHandler(), glGetUniformLocation(_ssaoShaderProgram.GetHandler(), "gBuffer.depth"), TextureBindingPoint::depth);
    glProgramUniform2i(_ssaoShaderProgram.GetHandler(), glGetUniformLocation(_ssaoShaderProgram.GetHandler(), "occlusionTextureSize"), _screenWidth, _screenHeight);

    // samples
    GenerateSamples(64, Vector2f{ 0.1f, 1.0f });

    // random vector texture
    auto randomVectorTextureSize = 4u;
    GenerateRandomTexture(randomVectorTextureSize);
    glProgramUniform1i(_ssaoShaderProgram.GetHandler(), glGetUniformLocation(_ssaoShaderProgram.GetHandler(), "randomVectorTex"), TextureBindingPoint::randomVector);
    glProgramUniform1i(_ssaoShaderProgram.GetHandler(), glGetUniformLocation(_ssaoShaderProgram.GetHandler(), "randomVectorTexSize"), static_cast<Float32>(randomVectorTextureSize));

    _lightingShaderProgram.SendToCard();
    glProgramUniform1i(_lightingShaderProgram.GetHandler(), glGetUniformLocation(_lightingShaderProgram.GetHandler(), "gBuffer.diffuseEmissive"), TextureBindingPoint::diffuseEmissive);
    glProgramUniform1i(_lightingShaderProgram.GetHandler(), glGetUniformLocation(_lightingShaderProgram.GetHandler(), "gBuffer.specularShininess"), TextureBindingPoint::specularShininess);
    glProgramUniform1i(_lightingShaderProgram.GetHandler(), glGetUniformLocation(_lightingShaderProgram.GetHandler(), "gBuffer.normal"), TextureBindingPoint::normal);
    glProgramUniform1i(_lightingShaderProgram.GetHandler(), glGetUniformLocation(_lightingShaderProgram.GetHandler(), "gBuffer.occlusion"), TextureBindingPoint::occlusion);
    glProgramUniform1i(_lightingShaderProgram.GetHandler(), glGetUniformLocation(_lightingShaderProgram.GetHandler(), "gBuffer.depth"), TextureBindingPoint::depth);
    glProgramUniform1i(_lightingShaderProgram.GetHandler(), glGetUniformLocation(_lightingShaderProgram.GetHandler(), "randomVectorTexSize"), randomVectorTextureSize);


    auto vertexes = array<Vector2f, 4>{ Vector2f{ -1, -1 }, Vector2f{ 1, -1 }, Vector2f{ 1, 1 }, Vector2f{ -1, 1 } };
    glGenVertexArrays(1, &_screenQuadVao);
    glBindVertexArray(_screenQuadVao);

    glGenBuffers(1, &_screenQuadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, _screenQuadVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(vertexes), vertexes.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    auto error = glGetError();
}

auto Ssao::BindGBuffer() -> void {
    glBindFramebuffer(GL_FRAMEBUFFER, _ssaoFbo);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

auto Ssao::SsaoPass() -> void {
    // wireline mode not applicable to deferred pass
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glViewport(0, 0, screenWidth, screenHeight);

    _ssaoShaderProgram.Use();

    // unbind texture to be used
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);

    glActiveTexture(GL_TEXTURE0 + TextureBindingPoint::diffuseEmissive);
    glBindTexture(GL_TEXTURE_2D, _diffuseEmissiveBuffer);
    glActiveTexture(GL_TEXTURE0 + TextureBindingPoint::specularShininess);
    glBindTexture(GL_TEXTURE_2D, _specularShininessBuffer);
    glActiveTexture(GL_TEXTURE0 + TextureBindingPoint::normal);
    glBindTexture(GL_TEXTURE_2D, _ssaoNormalBuffer);
    glActiveTexture(GL_TEXTURE0 + TextureBindingPoint::randomVector);
    glBindTexture(GL_TEXTURE_2D, _randomVectorTexture);
    glActiveTexture(GL_TEXTURE0 + TextureBindingPoint::depth);
    glBindTexture(GL_TEXTURE_2D, _ssaoDepthBuffer);

    glBindVertexArray(_screenQuadVao);
    glDrawArrays(GL_QUADS, 0, 4);
    glBindVertexArray(0);

    // finish using textures, bind them back to fbo
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _diffuseEmissiveBuffer, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, _specularShininessBuffer, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, _ssaoNormalBuffer, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _ssaoDepthBuffer, 0);

    auto error = glGetError();
}

auto Ssao::LightingPass() -> void {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(_screenQuadVao);
    _lightingShaderProgram.Use();

    glActiveTexture(GL_TEXTURE0 + TextureBindingPoint::diffuseEmissive);
    glBindTexture(GL_TEXTURE_2D, _diffuseEmissiveBuffer);
    glActiveTexture(GL_TEXTURE0 + TextureBindingPoint::specularShininess);
    glBindTexture(GL_TEXTURE_2D, _specularShininessBuffer);
    glActiveTexture(GL_TEXTURE0 + TextureBindingPoint::normal);
    glBindTexture(GL_TEXTURE_2D, _ssaoNormalBuffer);
    glActiveTexture(GL_TEXTURE0 + TextureBindingPoint::occlusion);
    glBindTexture(GL_TEXTURE_2D, _ssaoOcclusionBuffer);
    glActiveTexture(GL_TEXTURE0 + TextureBindingPoint::depth);
    glBindTexture(GL_TEXTURE_2D, _ssaoDepthBuffer);

    glDrawArrays(GL_QUADS, 0, 4);
    glBindVertexArray(0);

    auto error = glGetError();
}

auto Ssao::InitSsaoFbo() -> void {

    // diffuseEmissive
    glGenTextures(1, &_diffuseEmissiveBuffer);
    glBindTexture(GL_TEXTURE_2D, _diffuseEmissiveBuffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, _screenWidth, _screenHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // specularShininess
    glGenTextures(1, &_specularShininessBuffer);
    glBindTexture(GL_TEXTURE_2D, _specularShininessBuffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, _screenWidth, _screenHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // normal
    glGenTextures(1, &_ssaoNormalBuffer);
    glBindTexture(GL_TEXTURE_2D, _ssaoNormalBuffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, _screenWidth, _screenHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // occlusion
    glGenTextures(1, &_ssaoOcclusionBuffer);
    glBindTexture(GL_TEXTURE_2D, _ssaoOcclusionBuffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, _screenWidth, _screenHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // depth
    glGenTextures(1, &_ssaoDepthBuffer);
    glBindTexture(GL_TEXTURE_2D, _ssaoDepthBuffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, _screenWidth, _screenHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // fbo
    glGenFramebuffers(1, &_ssaoFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _ssaoFbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _diffuseEmissiveBuffer, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, _specularShininessBuffer, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, _ssaoNormalBuffer, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, _ssaoOcclusionBuffer, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _ssaoDepthBuffer, 0);
    auto drawBuffers = array<GLenum, 4>{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(drawBuffers.size(), drawBuffers.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto error = glGetError();
}

auto Ssao::GenerateSamples(unsigned int sampleCount, Vector2f sampleDistanceRange) -> void {
    std::random_device randomDevice;
    auto randomFloats = std::uniform_real_distribution<Float32>(0.0, 1.0);
    auto randomEngine = std::default_random_engine{ randomDevice() };
    auto ssaoKernel = vector<Vector3f>{};
    for (auto i = 0u; i < sampleCount; ++i) {
        auto sample = Vector3f{
            randomFloats(randomEngine) * 2.0f - 1.0f,
            randomFloats(randomEngine) * 2.0f - 1.0f,
            randomFloats(randomEngine)
        };
        sample = Normalize(sample);
        sample = sample * randomFloats(randomEngine);

        auto scale = static_cast<Float32>(i) / sampleCount;
        scale = sampleDistanceRange(0) + (sampleDistanceRange(1) - sampleDistanceRange(0)) * scale * scale;
        sample = sample * scale;
        ssaoKernel.push_back(sample);
    }
    auto location = glGetUniformLocation(_ssaoShaderProgram.GetHandler(), "samples");
    glProgramUniform3fv(_ssaoShaderProgram.GetHandler(), location, sampleCount, reinterpret_cast<Float32*>(ssaoKernel.data()));
}

auto Ssao::GenerateRandomTexture(unsigned int textureSize) -> void {
    std::random_device randomDevice;
    auto randomFloats = std::uniform_real_distribution<Float32>(-1.0, 1.0);
    auto randomEngine = std::default_random_engine(randomDevice());
    auto randomVectors = vector<Vector3f>{};
    for (auto i = 0u; i < textureSize * textureSize; ++i) {
        auto randomVector = Vector3f{
            randomFloats(randomEngine),
            randomFloats(randomEngine),
            0
        };
        randomVectors.push_back(randomVector);
    }

    glGenTextures(1, &_randomVectorTexture);
    glBindTexture(GL_TEXTURE_2D, _randomVectorTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, textureSize, textureSize);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureSize, textureSize, GL_RGB, GL_FLOAT, randomVectors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    auto error = glGetError();
}

}