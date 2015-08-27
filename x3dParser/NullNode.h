#pragma once

#include <string>

#include "X3dNode.h"

using std::string;

namespace x3dParser {

class NullNode : public X3dNode {
public:
    auto SetAttribute(const std::string&, std::string&&) -> void override;
    auto AddChild(pNode) -> void override;
};

}