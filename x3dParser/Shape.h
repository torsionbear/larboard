#pragma once

#include "X3dNode.h"
#include "Appearance.h"
#include "IndexedFaceSet.h"

namespace x3dParser {

class Shape : public X3dNode {
public:
    auto SetAttribute(std::string const&, std::string&&) -> void override;
    auto AddChild(X3dNode *) -> void override;
    
    auto GetAppearance() const -> Appearance const*;
    auto GetIndexedFaceSet() const -> IndexedFaceSet const*;

private:
    Appearance * _appearance = nullptr;
    IndexedFaceSet * _indexedFaceSet = nullptr;
};

}