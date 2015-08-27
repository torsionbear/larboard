#include "Shape.h"

using std::unique_ptr;

namespace x3dParser {
    
auto Shape::SetAttribute(std::string const&, std::string&&) -> void {
}
    
auto Shape::AddChild(pNode child) -> void {
    if(typeid(*child) == typeid(Appearance)) {
        _appearance.reset(static_cast<Appearance*>(child.release()));
    } else if(typeid(*child) == typeid(IndexedFaceSet)) {
        _indexedFaceSet.reset(static_cast<IndexedFaceSet*>(child.release()));
    }
}

auto Shape::GetAppearance() -> unique_ptr<Appearance>& {
    return _appearance;
}

auto Shape::GetIndexedFaceSet() -> unique_ptr<IndexedFaceSet>& {
    return _indexedFaceSet;
}

}