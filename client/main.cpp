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

void Init(core::Scene & scene)
{	
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

void DrawOneFrame(core::Scene & scene)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	scene.Draw();
    glFlush();
}

int main()
{
    RenderWindow rw;
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
	

    while (rw.Step())
    {
        DrawOneFrame(*scene);
    }

    return 0;
}