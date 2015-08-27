#include "X3dNode.h"

#include <sstream>
#include <unordered_map>
#include <boost/algorithm/string.hpp>

#include "NullNode.h"
#include "X3d.h"
#include "Scene.h"
#include "Transform.h"
#include "Group.h"
#include "Shape.h"
#include "Appearance.h"
#include "ImageTexture.h"
#include "TextureTransform.h"
#include "Material.h"
#include "IndexedFaceSet.h"
#include "Coordinate.h"
#include "Normal.h"
#include "TextureCoordinate.h"
#include "ViewPoint.h"
#include "PointLight.h"

using std::unique_ptr;
using std::make_unique;
using std::vector;
using std::string;
using std::stringstream;
using std::unordered_map;
using std::getline;
using std::pair;

using boost::algorithm::is_any_of;
using boost::algorithm::trim_left_if;
using boost::algorithm::trim_right_if;

namespace x3dParser {

auto X3dNode::BuildNode(string&& s) -> unique_ptr<X3dNode> {
    trim_left_if(s, is_any_of("<"));
    trim_right_if(s, is_any_of("/> \r\n\t"));

    auto ss = stringstream{move(s)};
    auto nodeType = string{};
    ss >> nodeType;
    const auto m = unordered_map<string, unique_ptr<X3dNode>(*)()>{
        {"X3D", []()->unique_ptr<X3dNode>{return make_unique<X3d>();}},
        {"Scene", []()->unique_ptr<X3dNode>{return make_unique<Scene>();}},
        {"Transform", []()->unique_ptr<X3dNode>{return make_unique<Transform>();}},
        {"Group", []()->unique_ptr<X3dNode>{return make_unique<Group>();}},
        {"Shape", []()->unique_ptr<X3dNode>{return make_unique<Shape>();}},
        {"Appearance", []()->unique_ptr<X3dNode>{return make_unique<Appearance>();}},
        {"ImageTexture", []()->unique_ptr<X3dNode>{return make_unique<ImageTexture>();}},
        {"TextureTransform", []()->unique_ptr<X3dNode>{return make_unique<TextureTransform>();}},
        {"Material", []()->unique_ptr<X3dNode>{return make_unique<Material>();}},
        {"IndexedFaceSet", []()->unique_ptr<X3dNode>{return make_unique<IndexedFaceSet>();}},
        {"Coordinate", []()->unique_ptr<X3dNode>{return make_unique<Coordinate>();}},
        {"Normal", []()->unique_ptr<X3dNode>{return make_unique<Normal>();}},
        {"TextureCoordinate", []()->unique_ptr<X3dNode>{return make_unique<TextureCoordinate>();}},
        {"Viewpoint", []()->unique_ptr<X3dNode>{return make_unique<Viewpoint>();}},
		{"PointLight", []()->unique_ptr<X3dNode> {return make_unique<PointLight>(); }},
    };
    
    if(m.find(nodeType) == m.end()) {
        return make_unique<NullNode>();    
    }
    auto node = m.at(nodeType)();
    auto attributeName = string{};
    auto attributeValue = string{};
    while(getline(ss, attributeName, '=')) {
        trim_left_if(attributeName, is_any_of(" \r\n\t"));
        auto quote = char{};
        ss.get(quote);
        getline(ss, attributeValue, quote);

        node->SetAttribute(attributeName, move(attributeValue));
    }
    return node;    
}

X3dNode::X3dNode() = default;

auto X3dNode::SetDef(string&& def) -> void {
    _def = move(def);
}
    
auto X3dNode::GetDef() const -> const string& {
    return _def;
}

}