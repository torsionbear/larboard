#include "X3d.h"

using std::unique_ptr;

namespace x3dParser {
        
auto X3d::SetAttribute(std::string const&, std::string&&) -> void {
}
    
auto X3d::AddChild(pNode child) -> void {
    if(typeid(*child) == typeid(Scene)) {
        _scene.reset(static_cast<Scene*>(child.release()));
    }
}
    
auto X3d::GetScene() -> unique_ptr<Scene>& {
    return _scene;
}

}