#include "Scene.h"

namespace x3dParser {
    
auto Scene::SetAttribute(std::string const&, std::string&&) -> void {
}

auto Scene::AddChild(X3dNode * child) -> void {
    if(typeid(*child) == typeid(Transform)) {
        _transform.push_back(static_cast<Transform*>(child));
    }
}

auto Scene::GetTransform() const -> std::vector<Transform *> const& {
    return _transform;
}

}
