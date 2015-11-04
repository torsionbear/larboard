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

auto LoadScene0() -> std::unique_ptr<core::Scene> {
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

	auto scene = LoadScene3();
	scene->PrepareForDraw();

	auto lastX = 0.0f;
	auto lastY = 0.0f;
	auto status = 0; // 0:none; 1:rotate; 2:pan;
    auto pickPoint = core::Point4f{ 0, 0, 0, 1 };
	rw.RegisterInputHandler([
        &pickPoint, &scene, &lastX, &lastY, &status, widthInverse = 1.0f/static_cast<float>(width), heightInverse = 1.0f / static_cast<float>(height)]
        (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
		case WM_LBUTTONDOWN:
			break;
		case WM_SIZE:
			break; 
        case WM_MBUTTONDOWN:
        {
            POINT point;
            point.x = GET_X_LPARAM(lParam);
            point.y = GET_Y_LPARAM(lParam);
            //ScreenToClient(hWnd, &point);
            auto x = static_cast<float>(point.x) * widthInverse;
            auto y = static_cast<float>(point.y) * heightInverse;
            auto ray = scene->GetActiveCamera()->GetRayTo(core::Vector2f{ x, y });
            if (scene->Picking(ray)) {
                pickPoint = ray.GetHead();
            } else {
                pickPoint = core::Point4f{ 0, 0, 0, 1 };
            }
            break;
        }
		case WM_MOUSEWHEEL:	// mouse wheel: zoom
        {
            POINT point;
            point.x = GET_X_LPARAM(lParam);
            point.y = GET_Y_LPARAM(lParam);
            // unlike WM_MBUTTONDOWN, WM_MOUSEWHEEL carries screen-based coordinates in lParam. Need to convert them to client-based coordinates
            ScreenToClient(hWnd, &point);
            auto x = static_cast<float>(point.x) * widthInverse;
            auto y = static_cast<float>(point.y) * heightInverse;

            auto ray = scene->GetActiveCamera()->GetRayTo(core::Vector2f{ x, y });
            auto step = 0.01f;
            if (scene->Picking(ray)) {
                step = ray.length * 0.0008f;
            }

            status = 0;
            scene->GetActiveCamera()->Translate(ray.direction * GET_WHEEL_DELTA_WPARAM(wParam) * step);

            break;
        }
		case WM_MOUSEMOVE:
		{
            POINT point;
            point.x = GET_X_LPARAM(lParam);
            point.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &point);
            auto x = static_cast<float>(point.x) * widthInverse;
            auto y = static_cast<float>(point.y) * heightInverse;

			auto * camera = scene->GetActiveCamera();
			if (wParam == MK_MBUTTON) {	// middle mouse button: rotate
				if (status == 1) {
                    camera->Rotate(pickPoint, core::Vector4f{ 0, 0, 1, 0 }, static_cast<float>(-(x - lastX)) * 5);
					camera->Rotate(pickPoint, camera->GetRightDirection(), static_cast<float>(-(y - lastY)) * 5);
				}
				lastX = x;
				lastY = y;
				status = 1;
			} else if ((wParam & MK_MBUTTON) && (wParam & MK_SHIFT)) {	// middle mouse button & shift key: pan
				if (status == 2) {
                    auto pickPointVector = static_cast<core::Vector4f>(pickPoint - camera->GetPosition());
                    auto panMultiplier = DotProduct(pickPointVector, camera->GetForwardDirection()) / camera->GetNearPlane();
					camera->Leftward(static_cast<float>(x - lastX) * camera->GetHalfWidth() * 2 * panMultiplier);
					camera->Upward(static_cast<float>(y - lastY) * camera->GetHalfHeight() * 2 * panMultiplier);
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