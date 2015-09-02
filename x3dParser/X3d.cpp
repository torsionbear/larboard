#include "X3d.h"

using std::unique_ptr;

namespace x3dParser {
        
auto X3d::SetAttribute(std::string const&, std::string&&) -> void {
}
    
auto X3d::AddChild(X3dNode * child) -> void {
    if(typeid(*child) == typeid(Scene)) {
        _scene = static_cast<Scene*>(child);
    }
}
    
auto X3d::GetScene() const -> Scene const* {
    return _scene;
}

}