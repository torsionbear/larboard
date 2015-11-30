#include "Scene.h"

#include <algorithm>
#include <stack>

#include <GL/glew.h>

using std::string;
using std::make_unique;
using std::array;

namespace core {

Scene::Scene(unsigned int width, unsigned int height)
    : _screenWidth(width)
    , _screenHeight(height) {
    // todo: send in resourceManager by argument
    _resourceManager = make_unique<ResourceManager>();
    _staticModelGroup = make_unique<StaticModelGroup>(_resourceManager.get());
    _deferredPassShaderProgram = ShaderProgram("shader/deferred_v.shader", "shader/deferred_f.shader");
}

Scene::~Scene() {
    glDeleteFramebuffers(1, &_fbo);
    glDeleteBuffers(1, &_cameraUbo);
    glDeleteBuffers(1, &_lightUbo);
    glDeleteFramebuffers(1, &_fbo);
}

auto Scene::CreateCamera() -> Camera * {
	_cameras.push_back(make_unique<Camera>());
	return _cameras.back().get();
}

auto Scene::CreateAmbientLight() -> AmbientLight * {
    _ambientLights.push_back(make_unique<AmbientLight>());
    return _ambientLights.back().get();
}

auto Scene::CreateDirectionalLight() -> DirectionalLight * {
	_directionalLights.push_back(make_unique<DirectionalLight>());
	return _directionalLights.back().get();
}

auto Scene::CreatePointLight() -> PointLight * {
	_pointLights.push_back(make_unique<PointLight>());
	return _pointLights.back().get();
}

auto Scene::CreateSpotLight() -> SpotLight * {
	_spotLights.push_back(make_unique<SpotLight>());
	return _spotLights.back().get();
}

auto Scene::Stage(Movable * movable) -> void {
	movable->AttachTo(_root);
}

auto Scene::Unstage(Movable * movable) -> void {
	movable->DetachFrom();
}

auto Scene::GetActiveCamera() const -> Camera * {
	return _cameras.front().get();
}

auto Scene::CreateTerrain(vector<string> && diffuseMapFiles, string const& heightMap) -> void {
    _terrain = make_unique<Terrain>(move(diffuseMapFiles), heightMap);
}

auto Scene::CreateSkyBox(std::array<std::string, 6>&& filenames) -> void {
    _skyBox = make_unique<SkyBox>(move(filenames));
}

auto Scene::Picking(Ray & ray) -> bool {
    auto ret = false;
    auto bvhRoot = _staticModelGroup->GetBvh()->GetRoot();
    auto nodeStack = std::stack<BvhNode *>{};
    nodeStack.push(bvhRoot);
    while (!nodeStack.empty()) {
        auto currentNode = nodeStack.top();
        nodeStack.pop();
        auto length = currentNode->GetAabb().IntersectRay(ray);
        if (length < 0) {
            continue;
        }
        if (currentNode->RightChild() != nullptr) {
            nodeStack.push(currentNode->RightChild());
        }
        if (currentNode->LeftChild() != nullptr) {
            nodeStack.push(currentNode->LeftChild());
        }
        if(currentNode->IsLeaf()) {
            auto shapes = currentNode->GetShapes();
            for (auto shape : shapes) {
                auto length = shape->GetAabb().IntersectRay(ray);
                if (length > 0) {
                    ray.length = length;
                    ret = true;
                }
            }
        }
    }
    return ret;
}

auto Scene::ToggleBackFace() -> void {
	_renderBackFace = !_renderBackFace;
	_renderBackFace ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
}

auto Scene::ToggleWireframe() -> void {
	_wireframeMode = !_wireframeMode;
}

auto Scene::ToggleBvh() -> void {
    _drawBvh = !_drawBvh;
}

auto Scene::PrepareForDraw() -> void {
    _cameraController = make_unique<CameraController>(this);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// setup ubo
	InitCameraData();
	LoadLightData();
    _staticModelGroup->PrepareForDraw();
    _staticModelGroup->GetBvh()->PrepareForDraw(*_resourceManager);
    if (nullptr != _skyBox) {
        _skyBox->PrepareForDraw();
    }
    if (nullptr != _terrain) {
        _terrain->PrepareForDraw(_cameras.front()->GetSightDistance());
    }
	// todo: sort shapes according to: 1. shader priority; 2. vbo/vao

    InitFbo();
    _deferredPassShaderProgram.SendToCard();
    glUseProgram(_deferredPassShaderProgram.GetHandler());
    glUniform1i(glGetUniformLocation(_deferredPassShaderProgram.GetHandler(), "gBuffer.color"), 0);
    glUniform1i(glGetUniformLocation(_deferredPassShaderProgram.GetHandler(), "gBuffer.normal"), 1);
    glUniform1i(glGetUniformLocation(_deferredPassShaderProgram.GetHandler(), "gBuffer.depth"), 2);

    glUniform4f(glGetUniformLocation(_deferredPassShaderProgram.GetHandler(), "viewport"), 0, 0, _screenWidth, _screenHeight);
    glUniform3f(glGetUniformLocation(_deferredPassShaderProgram.GetHandler(), "nearPlane"), _cameras.front()->GetHalfWidth(), _cameras.front()->GetHalfHeight(), _cameras.front()->GetNearPlane());

    auto vertexes = vector<Vector2f>{ Vector2f{-1, -1}, Vector2f{ 1, -1 }, Vector2f{ 1, 1 }, Vector2f{ -1, 1 } };
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

auto Scene::Draw() -> void {
    ForwardPass();
    DeferredPass();
}

auto Scene::ForwardPass() -> void {
    // switch to fbo
    UseFbo();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glViewport(0, 0, texWidth, texHeight);

    _wireframeMode ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    _cameraController->Step();

    // feed model independent data (camera) to shader via ubo
    LoadCameraData();
    UseCameraData(_cameras.front().get());

    if (nullptr != _skyBox) {
        _skyBox->Draw();
    }
    if (nullptr != _terrain) {
        _terrain->Draw(_cameras.front().get());
    }
    _staticModelGroup->Draw();
    if (_drawBvh) {
        _staticModelGroup->GetBvh()->Draw();
    }

    auto error = glGetError();
}

auto Scene::DeferredPass() -> void {
    // wireline mode not applicable to deferred pass
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    DetachFbo();
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

// store all camera's data in _cameraUbo. 
// if there's not need to split screen for multiple player, 
// maybe we should store only 1 camera's data in _cameraUbo
auto Scene::InitCameraData() -> void {
	glGenBuffers(1, &_cameraUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, _cameraUbo);
	glBufferData(GL_UNIFORM_BUFFER, Camera::ShaderData::Size() * _cameras.size(), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto Scene::LoadCameraData() -> void {
	auto cache = vector<unsigned char>(Camera::ShaderData::Size() * _cameras.size());
	auto offset = 0;
	for (auto & camera : _cameras) {
		auto * p = reinterpret_cast<Camera::ShaderData *>(&cache[offset]);
		*p = camera->GetShaderData();
		camera->SetUboOffset(offset);
		offset += Camera::ShaderData::Size();
	}
	glBindBuffer(GL_UNIFORM_BUFFER, _cameraUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, cache.size(), cache.data());
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto Scene::UseCameraData(Camera const * camera) -> void {
	glBindBufferRange(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Camera), _cameraUbo, camera->GetUboOffset(), sizeof(Camera::ShaderData));
}

auto Scene::LoadLightData() -> void {
	auto data = LightShaderData{};

    data.ambientLight = _ambientLights.front()->GetShaderData();

	data.directionalLightCount = _directionalLights.size();
	assert(data.directionalLightCount <= LightShaderData::MaxDirectionalLightCount);
	for (auto i = 0; i < data.directionalLightCount; ++i) {
		data.directionalLights[i] = _directionalLights[i]->GetShaderData();
	}

	data.pointLightCount = _pointLights.size();
	assert(data.pointLightCount <= LightShaderData::MaxPointLightCount);
	for (auto i = 0; i < data.pointLightCount; ++i) {
		data.pointLights[i] = _pointLights[i]->GetShaderData();
	}

	data.spotLightCount = _spotLights.size();
	assert(data.spotLightCount <= LightShaderData::MaxSpotLightCount);
	for (auto i = 0; i < data.spotLightCount; ++i) {
		data.spotLights[i] = _spotLights[i]->GetShaderData();
	}

	glGenBuffers(1, &_lightUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, _lightUbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(data), &data, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, GetIndex(UniformBufferType::Light), _lightUbo);
}

auto Scene::InitFbo() -> void {

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
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _fboColorBuffer, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, _fboNormalBuffer, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _fboDepthBuffer, 0);
    auto drawBuffers = array<GLenum, 2>{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, };
    glDrawBuffers(drawBuffers.size(), drawBuffers.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto error = glGetError();
}

auto Scene::DetachFbo() -> void {
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

auto Scene::UseFbo() -> void {
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(fboStatus == GL_FRAMEBUFFER_COMPLETE);
}

}