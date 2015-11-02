#include <GL/glew.h>
#include <iostream>
#include <algorithm>
#include <chrono>

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

auto LoadScene() -> std::unique_ptr<core::Scene> {
    auto scene = make_unique<core::Scene>();
	x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/square/square.x3d", scene.get()).Read();
	return move(scene);
}

auto LoadScene1() -> std::unique_ptr<core::Scene> {
    auto scene = make_unique<core::Scene>();
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/square2/square2.x3d", scene.get()).Read();
    return move(scene);
}

auto LoadScene2() -> std::unique_ptr<core::Scene> {
    auto scene = make_unique<core::Scene>();
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/8/8.x3d", scene.get()).Read();
	return move(scene);
}

auto LoadScene3() -> std::unique_ptr<core::Scene> {
    auto scene = make_unique<core::Scene>();
    x3dParser::X3dReader("D:/torsionbear/working/larboard/Modeling/xsh/xsh_00.x3d", scene.get()).Read();

    auto plainProgram = scene->GetStaticModelGroup().CreateShaderProgram("shader/plain.vert", "shader/plain.frag");
    for (auto & shape : scene->GetStaticModelGroup().GetShapes()) {
        shape->SetShaderProgram(plainProgram);
    }
    return move(scene);
}

int main()
{
	RenderWindow rw{};
    rw.Create(800, 600, L"RenderWindow");
	MessageLogger::Log(MessageLogger::Info, std::string((const char*)glGetString(GL_VERSION)));
	MessageLogger::Log(MessageLogger::Info, std::string((const char*)glGetString(GL_RENDERER)));

    if (glewInit())
    {
		MessageLogger::Log(MessageLogger::Error, "Unable to initialize GLEW ... exiting");
        exit(EXIT_FAILURE);
    }

	auto scene = LoadScene2();
	scene->PrepareForDraw();

	auto lastX = -1; 
	auto lastY = -1;
	auto status = 0; // 0.:none; 1:rotate; 2:pan;
	rw.RegisterInputHandler([&scene, &lastX, &lastY, &status](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
		case WM_LBUTTONDOWN:
			break;
		case WM_SIZE:
			break; 
		case WM_MOUSEWHEEL:	// mouse wheel: zoom
			status = 0;
			scene->GetActiveCamera()->Forward(GET_WHEEL_DELTA_WPARAM(wParam) * 0.002f);
			break;
		case WM_MOUSEMOVE:
		{
			auto x = GET_X_LPARAM(lParam);
			auto y = GET_Y_LPARAM(lParam);
			auto * camera = scene->GetActiveCamera();
			if (wParam == MK_MBUTTON) {	// middle mouse button: rotate
				if (status == 1) {
					camera->Rotate(0.0f, 0.0f, 1.0f, static_cast<float>(-(x - lastX)) / 150.0f);
					camera->Rotate(camera->GetRightDirection(), static_cast<float>(-(y - lastY)) / 150.0f);
				}
				lastX = x;
				lastY = y;
				status = 1;
			} else if ((wParam & MK_MBUTTON) && (wParam & MK_SHIFT)) {	// middle mouse button & shift key: pan
				if (status == 2) {
					camera->Leftward(static_cast<float>(x - lastX) / 100.0f);
					camera->Upward(static_cast<float>(y - lastY) / 100.0f);
				}
				lastX = x;
				lastY = y;
				status = 2;
			} else {
				status = 0;
			}
			break;
		}
		case WM_KEYDOWN:
			switch(wParam) {
			case VK_ESCAPE:
				break;
			case VK_F1:
				scene->ToggleBackFace();
				break;
			case VK_F2:
				scene->ToggleWireframe();
				break;
			case 0x57:	// W key
				scene->GetActiveCamera()->Forward(0.2f);
				break;
			case 0x41:	// A key
				scene->GetActiveCamera()->Leftward(0.2f);
				break;
			case 0x53:	// S key
				scene->GetActiveCamera()->Backward(0.2f);
				break;
			case 0x44:	// D key
				scene->GetActiveCamera()->Rightward(0.2f);
				break;
			case VK_SPACE:
				break;
			case 0x49:	// I key
				break;
			case 0x4B:	// K key
				break;
			default:
				break;
			}
			break;
		}
	});

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