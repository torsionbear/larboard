#include "Skybox.h"

#include <GL/glew.h>

namespace core {
SkyBox::~SkyBox() {
    glDeleteBuffers(1, &_veo);
    glDeleteBuffers(1, &_vbo);
    glDeleteVertexArrays(1, &_vao);
}
auto SkyBox::PrepareForDraw() -> void {
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glGenBuffers(1, &_veo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _veo);

    glBufferData(GL_ARRAY_BUFFER, _vertexData.size() * sizeof(Vector3f), _vertexData.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indexData.size() * sizeof(unsigned int), _indexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    _cubeMap->SendToCard();
    _shaderProgram->SendToCard();
}
auto SkyBox::Draw() -> void {
    _shaderProgram->Use();
    _cubeMap->Use();
    glBindVertexArray(_vao);

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, _indexData.size(), GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(0));
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

}