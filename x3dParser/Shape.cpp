#include "Shape.h"

using std::unique_ptr;

namespace x3dParser {
    
auto Shape::SetAttribute(std::string const&, std::string&&) -> void {
}
    
auto Shape::AddChild(X3dNode * child) -> void {
    if(typeid(*child) == typeid(Appearance)) {
        _appearance = static_cast<Appearance*>(child);
    } else if(typeid(*child) == typeid(IndexedFaceSet)) {
        _indexedFaceSet = static_cast<IndexedFaceSet*>(child);
    }
}

auto Shape::GetAppearance() const -> Appearance const* {
    return _appearance;
}

auto Shape::GetIndexedFaceSet() const -> IndexedFaceSet const* {
    return _indexedFaceSet;
}

}