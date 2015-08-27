#include "NullNode.h"

using std::string;

namespace x3dParser {

auto NullNode::SetAttribute(string const&, string&&) -> void {
}
    
auto NullNode::AddChild(pNode) -> void {
}

}