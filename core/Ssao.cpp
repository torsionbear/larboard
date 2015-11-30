#include "Ssao.h"

#include <array>

#include <GL/glew.h>

#include "Matrix.h"

using std::array;

namespace core {

Ssao::Ssao(unsigned int width, unsigned int height)
    : _screenWidth(width)
    , _screenHeight(height)
    , _deferredPassShaderProgram("shader/deferred_v.shader", "shader/deferred_f.shader") {
}

Ssao::~Ssao() {
    glDeleteFramebuffers(1, &_gBuffer);
}

auto Ssao::PrepareForDraw() -> void {
    InitFbo();
    _deferredPassShaderProgram.SendToCard();
    glUseProgram(_deferredPassShaderProgram.GetHandler());
    glUniform1i(glGetUniformLocation(_deferredPassShaderProgram.GetHandler(), "gBuffer.color"), 0);
    glUniform1i(glGetUniformLocation(_deferredPassShaderProgram.GetHandler(), "gBuffer.normal"), 1);
    glUniform1i(glGetUniformLocation(_deferredPassShaderProgram.GetHandler(), "gBuffer.depth"), 2);

    auto vertexes = array<Vector2f, 4>{ Vector2f{ -1, -1 }, Vector2f{ 1, -1 }, Vector2f{ 1, 1 }, Vector2f{ -1, 1 } };
    glGenVertexArrays(1, &_deferredPassVao);
    glBindVertexArray(_deferredPassVao);

    glGenBuffers(1, &_deferredPassVbo);
    glBindBuffer(GL_ARRAY_BUFFER, _deferredPassVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(vertexes), vertexes.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    auto error = glGetError();
}

auto Ssao::BindGBuffer() -> void {
    glBindFramebuffer(GL_FRAMEBUFFER, _gBuffer);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

auto Ssao::UnbindGBuffer() -> void {
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

auto Ssao::DeferredPass() -> void {
    // wireline mode not applicable to deferred pass
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    UnbindGBuffer();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glViewport(0, 0, screenWidth, screenHeight);
    glBindVertexArray(_deferredPassVao);

    _deferredPassShaderProgram.Use();

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, _fboColorBuffer);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, _fboNormalBuffer);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, _fboDepthBuffer);

    glDrawArrays(GL_QUADS, 0, 4);
    glBindVertexArray(0);

    auto error = glGetError();
}

auto Ssao::InitFbo() -> void {

    // color
    glGenTextures(1, &_fboColorBuffer);
    glBindTexture(GL_TEXTURE_2D, _fboColorBuffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, _screenWidth, _screenHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // normal
    glGenTextures(1, &_fboNormalBuffer);
    glBindTexture(GL_TEXTURE_2D, _fboNormalBuffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8_SNORM, _screenWidth, _screenHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // depth
    glGenTextures(1, &_fboDepthBuffer);
    glBindTexture(GL_TEXTURE_2D, _fboDepthBuffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, _screenWidth, _screenHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // fbo
    glGenFramebuffers(1, &_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _gBuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _fboColorBuffer, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, _fboNormalBuffer, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _fboDepthBuffer, 0);
    auto drawBuffers = array<GLenum, 2>{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, };
    glDrawBuffers(drawBuffers.size(), drawBuffers.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto error = glGetError();
}

}