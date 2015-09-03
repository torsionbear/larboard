#include <GL/glew.h>
#include <iostream>
#include <algorithm>

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

auto Init(core::Scene & scene) -> void {	
	auto model = scene.CreateModel();
	auto * shape = scene.CreateShape(model);
	auto v = glGetString(GL_VERSION);

	// texture
	auto * texture = scene.CreateTexture("texture0", "D:\\torsionbear\\working\\larboard\\Modeling\\square\\pedobear.png");
	//core::Texture texture{ "D:\\torsionbear\\working\\larboard\\Modeling\\square\\pedobear.png" };
	shape->AddTexture(texture);

	// vertex
	auto * mesh = scene.CreateMesh();
	mesh->SetVertexData(std::vector<core::Vertex>{
		{ { -0.90f, -0.90f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { 0.85f, -0.90f, 0.0f },	{ 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { -0.90f, 0.85f, 0.0f },	{ 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { 0.90f, -0.85f, 0.0f },	{ 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { 0.90f, 0.90f, 0.0f },	{ 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { -0.85f, 0.90f, 0.0f},	{ 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
	});
	shape->SetMesh(mesh);

	// shader program
	auto shaderProgram = scene.CreateShaderProgram("default.vert", "default.frag");
	shape->SetShaderProgram(shaderProgram);
}

auto DrawOneFrame(core::Scene & scene) -> void {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene.Draw();
    glFlush();
}

auto UpdateScene(core::Scene & scene) -> void {

}

int main()
{
	RenderWindow rw{};
    rw.Create(800, 600, L"RenderWindow");

    if (glewInit())
    {
		MessageLogger::Log(MessageLogger::Error, "Unable to initialize GLEW ... exiting");
        exit(EXIT_FAILURE);
    }

	auto scene= x3dParser::X3dReader::Read("D:/torsionbear/working/larboard/Modeling/8/8.x3d");
	//auto scene = std::make_unique<core::Scene>();
	//Init(*scene);
	scene->SendToCard();

	rw.RegisterKeyHandler([&scene](int keyCode) {
		switch (keyCode) {
		case VK_ESCAPE:
			break;
		case VK_F2:
			break;
		case 0x57:	// W key
			scene->GetActiveCamera()->Forward(0.2);
			break;
		case 0x41:	// A key
			scene->GetActiveCamera()->Leftward(0.2);
			break;
		case 0x53:	// S key
			scene->GetActiveCamera()->Backward(0.2);
			break;
		case 0x44:	// D key
			scene->GetActiveCamera()->Rightward(0.2);
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
	});
	

    while (rw.Step())
    {
		UpdateScene(*scene);
        DrawOneFrame(*scene);
    }

    return 0;
}