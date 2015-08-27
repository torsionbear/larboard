#include "Scene.h"

namespace x3dParser {
    
auto Scene::SetAttribute(std::string const&, std::string&&) -> void {
}

auto Scene::AddChild(pNode child) -> void {
    if(typeid(*child) == typeid(Transform)) {
        _transform.emplace_back(static_cast<Transform*>(child.release()));
    }
}

auto Scene::GetTransform() -> std::vector<std::unique_ptr<Transform>>& {
    return _transform;
}

}
