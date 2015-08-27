#pragma once

#include "X3dNode.h"
#include "Appearance.h"
#include "IndexedFaceSet.h"

namespace x3dParser {

class Shape : public X3dNode {
public:
    auto SetAttribute(const std::string&, std::string&&) -> void override;
    auto AddChild(pNode) -> void override;
    
    auto GetAppearance() -> std::unique_ptr<Appearance>&;
    auto GetIndexedFaceSet() -> std::unique_ptr<IndexedFaceSet>&;

private:
    std::unique_ptr<Appearance> _appearance = nullptr;
    std::unique_ptr<IndexedFaceSet> _indexedFaceSet = nullptr;
};

}