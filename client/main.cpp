#include <GL/glew.h>
#include <iostream>
#include <algorithm>
#include <chrono>

#include "InputHandler.h"

#include "renderSystem/RenderWindow.h"
#include "core/scene.h"
#include "core/Texture.h"
#include "core/ShaderProgram.h"
#include "core/PngReader.h"
#include "core/MessageLogger.h"
#include "core/Mesh.h"
#include "x3dParser/X3dReader.h"

using core::ShaderProgram;
using core::MessageLogger;

using std::max;

auto DrawOneFrame(core::Scene & scene) -> void {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene.Draw();
    glFlush();
}

auto UpdateScene(core::Scene & scene) -> void {

}

auto LoadScene0() -> std::unique_ptr<core::Scene> {
    auto scene = make_unique<core::Scene>();
	x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/square/square.x3d", scene.get()).Read();
    scene->CreateAmbientLight()->SetColor(core::Vector4f{ 0.1f, 0.1f, 0.1f, 1.0f });
    scene->CreateSkyBox(std::array<std::string, 6>{"media/skybox/RT.png", "media/skybox/LF.png", "media/skybox/FT.png", "media/skybox/BK.png", "media/skybox/UP.png", "media/skybox/DN.png", });
    return move(scene);
}

auto LoadScene1() -> std::unique_ptr<core::Scene> {
    auto scene = make_unique<core::Scene>();
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/square2/square2.x3d", scene.get()).Read();
    scene->CreateAmbientLight()->SetColor(core::Vector4f{ 0.1f, 0.1f, 0.1f, 1.0f });
    scene->CreateSkyBox(std::array<std::string, 6>{"media/skybox/mt_rt.png", "media/skybox/mt_lf.png", "media/skybox/mt_ft.png", "media/skybox/mt_bk.png", "media/skybox/mt_up.png", "media/skybox/mt_dn.png", });
    scene->CreateTerrain(2, core::Vector2i{ -8, -8 }, core::Vector2i{ 16, 16 }, "media/terrain/diffuseMap.png", "media/terrain/heightMap.png");
    return move(scene);
}

auto LoadScene2() -> std::unique_ptr<core::Scene> {
    auto scene = make_unique<core::Scene>();
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/8/8.x3d", scene.get()).Read();
    scene->CreateAmbientLight()->SetColor(core::Vector4f{ 0.1f, 0.1f, 0.1f, 1.0f });
    scene->CreateSkyBox(std::array<std::string, 6>{"media/skybox/RT.png", "media/skybox/LF.png", "media/skybox/FT.png", "media/skybox/BK.png", "media/skybox/UP.png", "media/skybox/DN.png", });
	return move(scene);
}

auto LoadScene3() -> std::unique_ptr<core::Scene> {
    auto scene = make_unique<core::Scene>();
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/xsh/xsh_00.x3d", scene.get()).Read();
    scene->CreateAmbientLight()->SetColor(core::Vector4f{ 0.0f, 0.0f, 0.0f, 1 });
    scene->CreateSkyBox(std::array<std::string, 6>{"media/skybox/RT.png", "media/skybox/LF.png", "media/skybox/FT.png", "media/skybox/BK.png", "media/skybox/UP.png", "media/skybox/DN.png", });

    auto plainProgram = scene->GetStaticModelGroup().CreateShaderProgram("shader/noTexture.vert", "shader/noTexture.frag");
    for (auto & shape : scene->GetStaticModelGroup().GetShapes()) {
        shape->SetShaderProgram(plainProgram);
    }
    return move(scene);
}

int main()
{
	RenderWindow rw{};
    auto width = 800;
    auto height = 600;
    rw.Create(width, height, L"RenderWindow");
	MessageLogger::Log(MessageLogger::Info, std::string((const char*)glGetString(GL_VERSION)));
	MessageLogger::Log(MessageLogger::Info, std::string((const char*)glGetString(GL_RENDERER)));

    if (glewInit())
    {
		MessageLogger::Log(MessageLogger::Error, "Unable to initialize GLEW ... exiting");
        exit(EXIT_FAILURE);
    }

	auto scene = LoadScene1();
	scene->PrepareForDraw();

	auto lastX = 0.0f;
	auto lastY = 0.0f;
	auto status = 0; // 0:none; 1:rotate; 2:pan;
    auto pickPoint = core::Point4f{ 0, 0, 0, 1 };
    rw.RegisterInputHandler(std::function<void(HWND, UINT, WPARAM, LPARAM)>(InputHandler(scene.get(), width, height)));

	rw.SetCaption(L"newCaption");
	auto fpsCount = 0u;
	auto clock = std::chrono::system_clock();
	auto timePoint = clock.now();
    while (rw.Step())
    {
		if (++fpsCount == 100u) {
			fpsCount = 0;
			auto lastTimePoint = timePoint;
			timePoint = clock.now();
			auto timeSpan = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint - lastTimePoint);
			auto fps = 100000.0f / static_cast<float>(timeSpan.count());
			rw.SetCaption(L"FPS: " + std::to_wstring(fps));
		}
		UpdateScene(*scene);
        DrawOneFrame(*scene);
    }

    return 0;
}