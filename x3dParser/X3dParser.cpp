#include "X3dParser.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <stack>
#include <assert.h>

#include <boost/algorithm/string.hpp>

using std::move;
using std::stringstream;
using std::vector;
using std::istream;
using std::remove_if;
using std::getline;
using std::stack;
using std::string;
using std::unique_ptr;

using boost::algorithm::starts_with;
using boost::algorithm::ends_with;
using boost::algorithm::is_any_of;
using boost::algorithm::trim_left_if;
using boost::algorithm::trim_right_if;

namespace x3dParser {

auto X3dParser::Parse(istream& is) -> unique_ptr<X3dNode> {
    auto tags = SplitToTags(is);
    
    auto shouldSkip = [](string const& s) -> bool {
        return starts_with(s, "<?xml") || starts_with(s, "<!DOCTYPE");
    };
    tags.erase(remove_if(tags.begin(), tags.end(), shouldSkip), tags.end());

    return BuildTree(tags);
}

auto X3dParser::SplitToTags(istream& is) -> vector<string> {
    vector<string> vec;    
    string str;
    while (getline(is, str, '>')) {
        trim_left_if(str, is_any_of(" \r\n\t"));
        str.push_back('>');
        vec.push_back(move(str));
    }
    return vec;
}

auto X3dParser::BuildTree(vector<string>& tags) -> unique_ptr<X3dNode> {
    auto nodeStack = stack<unique_ptr<X3dNode>>();
    for(auto& s : tags) {
        if(starts_with(s, "</")) {
            auto node = move(nodeStack.top());
            nodeStack.pop();
            if(nodeStack.empty()) {
                return node;
            }
            nodeStack.top()->AddChild(move(node));
        } else if(ends_with(s, "/>")) {
            nodeStack.top()->AddChild(X3dNode::BuildNode(move(s)));
        } else {
            nodeStack.push(X3dNode::BuildNode(move(s)));
        }
    }
    return nullptr;
}

}