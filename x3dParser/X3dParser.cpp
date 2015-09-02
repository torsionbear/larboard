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
using std::unordered_map;
using std::string;
using std::unique_ptr;
using std::make_unique;

using boost::algorithm::starts_with;
using boost::algorithm::ends_with;
using boost::algorithm::is_any_of;
using boost::algorithm::trim_left_if;
using boost::algorithm::trim_right_if;

namespace x3dParser {

auto X3dParser::Parse(istream& is) -> vector<unique_ptr<X3dNode>> {
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

auto X3dParser::ParseTag(std::string && s) -> X3dNode * {
	trim_left_if(s, is_any_of("<"));
	trim_right_if(s, is_any_of("/> \r\n\t"));

	auto ss = stringstream{ move(s) };
	auto nodeType = string{};
	ss >> nodeType;
	auto node = X3dNode::BuildNode(nodeType);
	auto attributeName = string{};
	auto attributeValue = string{};
	while (getline(ss, attributeName, '=')) {
		trim_left_if(attributeName, is_any_of(" \r\n\t"));
		auto quote = char{};
		ss.get(quote);

		getline(ss, attributeValue, quote);
		node->SetAttribute(attributeName, move(attributeValue));
	}
	_nodes.push_back(move(node));
	return _nodes.back().get();
}

auto X3dParser::BuildTree(vector<string>& tags) -> vector<unique_ptr<X3dNode>> {
    auto nodeStack = stack<X3dNode *>();
    for(auto& s : tags) {
        if(starts_with(s, "</")) {
            nodeStack.pop();
		} else {
			auto isSelfClosed = ends_with(s, "/>");
			auto * node = ParseTag(move(s));
			if (!nodeStack.empty()) {
				nodeStack.top()->AddChild(node);
			}
			if (!isSelfClosed) {
				nodeStack.push(node);
			}
        }
    }
    return move(_nodes);
}

}