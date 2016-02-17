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

auto static const width = 800;
auto static const height = 600;

auto LoadScene_dx0(core::Scene * scene) -> void {
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/square/square.x3d").Read(scene);
}

auto LoadScene_dx1(core::Scene * scene) -> void {
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/square2/square2.x3d").Read(scene);
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
    auto const width = 800;
    auto const height = 600;


    auto scene = make_unique<core::Scene>();
    LoadScene_dx1(scene.get());
    scene->Load();

    d3d12RenderSystem::RenderSystem renderSystem;
    renderSystem.Init(width, height);

    auto & resourceManager = renderSystem.GetResourceManager();
    auto & renderer = renderSystem.GetRenderer();
    auto & renderWindow = renderSystem.GetRenderWindow();
    auto cameraController = core::CameraController(scene.get());

    auto inputHandler = InputHandler(&renderer, scene.get(), &cameraController, width, height);
    renderSystem.GetRenderWindow().RegisterInputHandler(std::function<void(HWND, UINT, WPARAM, LPARAM)>(inputHandler));

    // load
    resourceManager.LoadBegin(1, 1, scene->GetStaticModelGroup().GetMeshes().size(), scene->GetStaticModelGroup().GetModels().size(), scene->GetStaticModelGroup()._textures.size());
    resourceManager.CreateDepthStencil(width, height);
    resourceManager.LoadCamera(scene->GetActiveCamera(), 1);

    auto meshes = vector<core::Mesh *>{};
    for (auto & m : scene->GetStaticModelGroup().GetMeshes()) {
        meshes.push_back(m.get());
    }
    resourceManager.LoadMeshes(meshes.data(), meshes.size(), sizeof(core::Vertex));

    auto models = vector<core::Model *>{};
    for (auto & model : scene->GetStaticModelGroup().GetModels()) {
        models.push_back(model.get());
    }
    resourceManager.LoadModels(models.data(), models.size());

    for (auto & t : scene->GetStaticModelGroup()._textures) {
        resourceManager.LoadTexture(t.second.get());
    }
    resourceManager.LoadEnd();

    while (renderWindow.Step()) {
        resourceManager.PrepareResource();
        // update
        cameraController.Step();
        resourceManager.UpdateCamera(*scene->GetActiveCamera());
        renderer.DrawBegin();
        renderer.UseCamera(scene->GetActiveCamera());
        // render
        for (auto & shape : scene->GetStaticModelGroup().GetShapes()) {
            renderer.RenderShape(shape.get());
        }
        renderer.DrawEnd();
    }

    return 0;
}

int main()
{
    //return main_gl();
    return main_dx();
}