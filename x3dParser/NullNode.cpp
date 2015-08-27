#include "NullNode.h"

using std::string;

namespace x3dParser {

auto NullNode::SetAttribute(const string&, string&&) -> void {
}
    
auto NullNode::AddChild(pNode) -> void {
}

}