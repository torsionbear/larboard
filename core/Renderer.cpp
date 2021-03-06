#include "Renderer.h"

#include <GL/glew.h>

namespace core {

auto Renderer::DrawScene(Renderer * renderer, Scene const * scene) -> void {
    renderer->DrawBegin();

    if (nullptr != scene->_skyBox) {
        renderer->RenderSkyBox(scene->_skyBox.get());
    }
    if (nullptr != scene->_terrain) {
        renderer->DrawTerrain(scene->_terrain.get());
    }
    for (auto const& shape : scene->_staticModelGroup->GetShapes()) {
        renderer->Render(shape.get());
    }
    if (scene->_drawBvh) {
        auto const& aabbs = scene->_staticModelGroup->GetBvh()->GetAabbs();
        for (auto aabb : aabbs) {
            renderer->RenderAabb(aabb);
        }
    }
    renderer->DrawEnd();
}

auto Renderer::Prepare() -> void {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

auto Renderer::DrawBegin() -> void {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glViewport(0, 0, texWidth, texHeight);
    _wireframeMode ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

auto Renderer::DrawEnd() -> void {
}

auto Renderer::ToggleWireframe() -> void {
    _wireframeMode = !_wireframeMode;
}

auto Renderer::ToggleBackFace() -> void {
    _renderBackFace = !_renderBackFace;
    _renderBackFace ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
}

auto Renderer::Render(Shape const* shape) -> void {
    // shaderProgram
    shape->GetShaderProgram()->Use();
    //if (_currentShaderProgram != shape.GetShaderProgram()) {
    //    shape.GetShaderProgram()->Use();
    //    _currentShaderProgram = shape.GetShaderProgram();
    //}

    // transform
    auto model = shape->GetModel();
    glBindBufferRange(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Transform), model->GetUbo(), model->GetUboOffset(), Model::ShaderData::Size());

    // material
    auto material = shape->GetMaterial();
    glBindBufferRange(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Material), material->GetUbo(), material->GetUboOffset(), Material::ShaderData::Size());

    // texture
    for (auto & texture : shape->GetTextures()) {
        UseTexture(texture, texture->GetType());
    }

    // draw call
    auto mesh = shape->GetMesh();
    auto const& meshRenderData = mesh->GetRenderData();

    glBindVertexArray(meshRenderData._vao);
    glDrawElementsBaseVertex(GL_TRIANGLES, mesh->GetIndex().size(), GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(meshRenderData._indexOffset), meshRenderData._baseVertex);
    glBindVertexArray(0);

    auto error = glGetError();
    assert(error == GL_NO_ERROR);
}

auto Renderer::RenderAabb(Aabb const * aabb) -> void {
    auto const& shaderProgram = aabb->GetShaderProgram();
    shaderProgram->Use();

    auto location = glGetUniformLocation(shaderProgram->GetHandler(), "color");
    glUniform4f(location, 1.0f, 0.0f, 0.0f, 1.0f);

    auto const& renderData = aabb->GetRenderData();
    glBindVertexArray(renderData._vao);
    glDrawElementsBaseVertex(GL_LINES, 24, GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(renderData._indexOffset), renderData._baseVertex);
    glBindVertexArray(0);
}

auto Renderer::RenderSkyBox(SkyBox const * skyBox) -> void {
    skyBox->GetShaderProgram()->Use();

    UseCubeMap(skyBox->GetCubeMap());

    //draw call
    glBindVertexArray(skyBox->GetVao());

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(0));
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

auto Renderer::DrawTerrain(Terrain const * terrain) -> void {
    UseTextureArray(terrain->GetDiffuseMap(), TextureUsage::DiffuseTextureArray);
    UseTexture(terrain->GetHeightMap(), TextureUsage::HeightMap);
    terrain->GetShaderProgram()->Use();

    glBindVertexArray(terrain->GetVao());
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glDrawElementsInstanced(GL_PATCHES, 6, GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(0), terrain->GetTileCount());
    glBindVertexArray(0);

    // draw special tiles
    for (auto & mesh : terrain->GetSpecialTiles()) {
        auto const& renderData = mesh->GetRenderData();
        glBindVertexArray(renderData._vao);
        glDrawElementsBaseVertex(GL_PATCHES, mesh->GetIndex().size(), GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(renderData._indexOffset), renderData._baseVertex);
        glBindVertexArray(0);
    }
    auto error = glGetError();
    error = glGetError();
    assert(error == GL_NO_ERROR);
}

auto Renderer::UseCubeMap(CubeMap const * cubeMap) -> void {
    glActiveTexture(GL_TEXTURE0 + TextureUsage::CubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap->GetTexture());
}

auto Renderer::UseTextureArray(TextureArray const * textureArray, TextureUsage::TextureType type) -> void {
    glActiveTexture(GL_TEXTURE0 + type);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray->GetTexture());
}

auto Renderer::UseTexture(Texture const * texture, TextureUsage::TextureType type) -> void {
    glActiveTexture(GL_TEXTURE0 + type);
    glBindTexture(GL_TEXTURE_2D, texture->GetTexture());
}

}
