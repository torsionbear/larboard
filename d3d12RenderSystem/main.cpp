#include "RenderWindow.h"
#include "RenderSystem.h"

#include <memory>
#include <vector>

using namespace d3d12RenderSystem;

auto main() -> int {
    auto const width = 800;
    auto const height = 600;

    RenderWindow renderWindow{};
    renderWindow.Create(width, height, L"RenderWindow");

    auto renderSystem = RenderSystem{&renderWindow };

    renderSystem.Init();
    renderSystem.LoadBegin();

    //auto vertexData = std::array<float, 128>{};
    //auto indexData = std::array<unsigned int, 3>{0, 1, 2};
    //auto mesh = std::vector<std::unique_ptr<core::Mesh>>{ std::make_unique<core::Mesh>(vertexData, indexData)};
    //renderSystem.LoadMeshes(mesh);

    renderSystem.LoadEnd();

    while (renderWindow.Step()) {
        renderSystem.Update();
        renderSystem.RenderBegin();
        renderSystem.RenderEnd();
    }

    return 0;
}