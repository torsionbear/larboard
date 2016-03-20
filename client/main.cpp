#include <GL/glew.h>
#include <iostream>
#include <algorithm>
#include <chrono>

#include "InputHandler.h"

#include "renderSystem/RenderWindow.h"
#include "core/CameraController.h"
#include "core/scene.h"
#include "core/ResourceManager.h"
#include "core/Renderer.h"
#include "core/ssao.h"
#include "core/Texture.h"
#include "core/ShaderProgram.h"
#include "core/PngReader.h"
#include "core/MessageLogger.h"
#include "core/Mesh.h"
#include "x3dParser/X3dReader.h"
#include "d3d12RenderSystem/RenderSystem.h"

using core::ShaderProgram;
using core::MessageLogger;

using std::max;

auto static const width = 1024;
auto static const height = 768;

auto LoadScene_dx0(core::Scene * scene) -> void {
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/square/square_dx.x3d").Read(scene);
    scene->CreateAmbientLight()->SetColor(core::Vector4f{ 0.4f, 0.4f, 0.4f, 1.0f });
}

auto LoadScene_dx1(core::Scene * scene) -> void {
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/square2/square2.x3d").Read(scene);
    scene->CreateAmbientLight()->SetColor(core::Vector4f{ 0.4f, 0.4f, 0.4f, 1.0f });
    scene->CreateSkyBox("media/skybox/cloudy_noon.dds");

    scene->CreateTerrain({ "media/terrain/grass2.png", "media/terrain/dirt2.png", "media/terrain/rock2.png" }, "media/terrain/heightMap.png");
    scene->GetTerrain()->SetSightDistance(scene->GetActiveCamera()->GetFarPlane());
    scene->_terrain->Load(scene->GetActiveCamera()->GetFarPlane()); // still need to load terrain png texture for height calculation
    scene->GetTerrain()->SetTileSize(10);
    scene->GetTerrain()->SetHeightMapOrigin(core::Vector2i{ -30, -24 });
    scene->GetTerrain()->SetHeightMapSize(core::Vector2i{ 60, 60 });
    scene->GetTerrain()->SetDiffuseMapSize(core::Vector2i{ 5, 5 });

    auto terrainSpecialTileScene = make_unique<core::Scene>();

    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/xsh/xsh_01_terrainx3d.x3d").Read(terrainSpecialTileScene.get());
    scene->GetTerrain()->AddSpecialTiles(terrainSpecialTileScene->GetStaticModelGroup().AcquireShapes(), terrainSpecialTileScene->GetStaticModelGroup().AcquireMeshes());

}

auto LoadScene_dx2(core::Scene * scene) -> void {
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/8/8.x3d").Read(scene);
    scene->CreateAmbientLight()->SetColor(core::Vector4f{ 0.4f, 0.4f, 0.4f, 1.0f });
    scene->CreateSkyBox("media/skybox/mt.dds");
}

auto LoadScene_dx3(core::Scene * scene) -> void {
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/xsh/xsh_02/xsh_02_house.x3d").Read(scene);
    scene->CreateAmbientLight()->SetColor(core::Vector4f{ 0.6f, 0.6f, 0.6f, 1.0f });
    scene->CreateSkyBox("media/skybox/cloudy_noon.dds");

    scene->CreateTerrain({ "media/terrain/grass2.png", "media/terrain/dirt2.png", "media/terrain/rock2.png" }, "media/terrain/heightMap.png");
    scene->GetTerrain()->SetSightDistance(scene->GetActiveCamera()->GetFarPlane());
    scene->_terrain->Load(scene->GetActiveCamera()->GetFarPlane()); // still need to load terrain png texture for height calculation
    scene->GetTerrain()->SetTileSize(10);
    scene->GetTerrain()->SetHeightMapOrigin(core::Vector2i{ -30, -24 });
    scene->GetTerrain()->SetHeightMapSize(core::Vector2i{ 60, 60 });
    scene->GetTerrain()->SetDiffuseMapSize(core::Vector2i{ 5, 5 });

    auto terrainSpecialTileScene = make_unique<core::Scene>();

    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/xsh/xsh_01_terrainx3d.x3d").Read(terrainSpecialTileScene.get());
    scene->GetTerrain()->AddSpecialTiles(terrainSpecialTileScene->GetStaticModelGroup().AcquireShapes(), terrainSpecialTileScene->GetStaticModelGroup().AcquireMeshes());

}

auto LoadScene0(core::Scene * scene) -> void {
	x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/square/square.x3d").Read(scene);
    scene->CreateAmbientLight()->SetColor(core::Vector4f{ 0.1f, 0.1f, 0.1f, 1.0f });
    //scene->CreateSkyBox(std::array<std::string, 6>{"media/skybox/RT.png", "media/skybox/LF.png", "media/skybox/FT.png", "media/skybox/BK.png", "media/skybox/UP.png", "media/skybox/DN.png", });

}

auto LoadScene1(core::Scene * scene) -> void {
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/square2/square2.x3d").Read(scene);
    scene->CreateAmbientLight()->SetColor(core::Vector4f{ 0.1f, 0.1f, 0.1f, 1.0f });
    scene->CreateSkyBox(std::array<std::string, 6>{"media/skybox/mt_rt.png", "media/skybox/mt_lf.png", "media/skybox/mt_ft.png", "media/skybox/mt_bk.png", "media/skybox/mt_up.png", "media/skybox/mt_dn.png", });
}

auto LoadScene2(core::Scene * scene) -> void {
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/8/8.x3d").Read(scene);
    scene->CreateAmbientLight()->SetColor(core::Vector4f{ 0.1f, 0.1f, 0.1f, 1.0f });
    scene->CreateSkyBox(std::array<std::string, 6>{"media/skybox/RT.png", "media/skybox/LF.png", "media/skybox/FT.png", "media/skybox/BK.png", "media/skybox/UP.png", "media/skybox/DN.png", });
}

auto LoadScene3(core::Scene * scene) -> void {
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/xsh/xsh_02/xsh_02_house.x3d").Read(scene);

    scene->CreateAmbientLight()->SetColor(core::Vector4f{ 0.5f, 0.5f, 0.5f, 1 });
    scene->CreateSkyBox(std::array<std::string, 6>{"media/skybox/RT.png", "media/skybox/LF.png", "media/skybox/FT.png", "media/skybox/BK.png", "media/skybox/UP.png", "media/skybox/DN.png", });

    scene->CreateTerrain({ "media/terrain/grass2.png", "media/terrain/dirt2.png", "media/terrain/rock2.png" }, "media/terrain/heightMap.png");
    scene->GetTerrain()->SetTileSize(10);
    scene->GetTerrain()->SetHeightMapOrigin(core::Vector2i{ -30, -24 });
    scene->GetTerrain()->SetHeightMapSize(core::Vector2i{ 60, 60 });
    scene->GetTerrain()->SetDiffuseMapSize(core::Vector2i{ 5, 5 });

    auto terrainSpecialTileScene = make_unique<core::Scene>();

    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/xsh/xsh_01_terrainx3d.x3d").Read(terrainSpecialTileScene.get());
    scene->GetTerrain()->AddSpecialTiles(terrainSpecialTileScene->GetStaticModelGroup().AcquireShapes(), terrainSpecialTileScene->GetStaticModelGroup().AcquireMeshes());

}

int main_gl() {
    RenderWindow rw{};
    rw.Create(width, height, L"RenderWindow");
    MessageLogger::Log(MessageLogger::Info, std::string((const char*)glGetString(GL_VERSION)));
    MessageLogger::Log(MessageLogger::Info, std::string((const char*)glGetString(GL_RENDERER)));

    if (glewInit()) {
        MessageLogger::Log(MessageLogger::Error, "Unable to initialize GLEW ... exiting");
        exit(EXIT_FAILURE);
    }
    auto resourceManager = make_unique<core::ResourceManager>();
    auto renderer = make_unique<core::Ssao>(width, height);
    auto scene = make_unique<core::Scene>();
    auto cameraController = make_unique<core::CameraController>(scene.get());

    LoadScene3(scene.get());
    scene->Load();

    renderer->Prepare();
    resourceManager->LoadScene(scene.get());

    auto lastX = 0.0f;
    auto lastY = 0.0f;
    auto status = 0; // 0:none; 1:rotate; 2:pan;
    auto pickPoint = core::Point4f{ 0, 0, 0, 1 };

    auto inputHandler = InputHandler(renderer.get(), scene.get(), cameraController.get(), width, height);
    rw.RegisterInputHandler(std::function<void(HWND, UINT, WPARAM, LPARAM)>(inputHandler));

    rw.SetCaption(L"newCaption");
    auto fpsCount = 0u;
    auto clock = std::chrono::system_clock();
    auto timePoint = clock.now();
    while (rw.Step()) {
        if (++fpsCount == 100u) {
            fpsCount = 0;
            auto lastTimePoint = timePoint;
            timePoint = clock.now();
            auto timeSpan = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint - lastTimePoint);
            auto fps = 100000.0f / static_cast<float>(timeSpan.count());
            rw.SetCaption(L"FPS: " + std::to_wstring(fps));
        }

        cameraController->Step();
        core::ResourceManager::UpdateScene(resourceManager.get(), scene.get());
        core::Renderer::DrawScene(renderer.get(), scene.get());
    }

    return 0;
}

int main_dx() {

    auto scene = make_unique<core::Scene>();
    LoadScene_dx3(scene.get());
    scene->GetStaticModelGroup().Load();

    d3d12RenderSystem::RenderSystem renderSystem;
    renderSystem.Init(width, height);

    auto resourceManager = renderSystem.GetResourceManager();
    auto renderer = renderSystem.GetRenderer();
    auto & renderWindow = renderSystem.GetRenderWindow();
    auto cameraController = core::CameraController(scene.get());

    auto inputHandler = InputHandler(renderer, scene.get(), &cameraController, width, height);
    renderSystem.GetRenderWindow().RegisterInputHandler(std::function<void(HWND, UINT, WPARAM, LPARAM)>(inputHandler));
    if (scene->_skyBox != nullptr) {
        renderSystem.RegisterSkyBox(scene->_skyBox.get());
    }
    if (scene->_terrain != nullptr) {
        renderSystem.RegisterTerrain(scene->_terrain.get());
    }
    renderSystem.RegisterCamera(scene->GetActiveCamera());
    renderSystem.RegisterStaticModelGroup(scene->GetStaticModelGroup());
    for (auto & ambientLight : scene->_ambientLights) {
        renderSystem.RegisterAmbientLight(ambientLight.get());
    }
    for (auto & directionalLight : scene->_directionalLights) {
        renderSystem.RegisterDirectionalLight(directionalLight.get());
    }
    for (auto & pointLight : scene->_pointLights) {
        renderSystem.RegisterPointLight(pointLight.get());
    }
    for (auto & spotLight : scene->_spotLights) {
        renderSystem.RegisterSpotLight(spotLight.get());
    }
    renderSystem.Load();

    renderWindow.SetCaption(L"newCaption");
    auto fpsCount = 0u;
    auto clock = std::chrono::system_clock();
    auto timePoint = clock.now();
    while (renderWindow.Step()) {
        if (++fpsCount == 100u) {
            fpsCount = 0;
            auto lastTimePoint = timePoint;
            timePoint = clock.now();
            auto timeSpan = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint - lastTimePoint);
            auto fps = 100000.0f / static_cast<float>(timeSpan.count());
            renderWindow.SetCaption(L"FPS: " + std::to_wstring(fps));
        }
        // update
        cameraController.Step();
        renderSystem.Update();
        // draw
        renderSystem.Draw();
    }
    return 0;
}

int main()
{
    //return main_gl();
    return main_dx();
}