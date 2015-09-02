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

auto X3dNode::BuildNode(string const& nodeType) -> unique_ptr<X3dNode> {
	if (nodeType == "X3D") {
		return make_unique<X3d>();
	} else if (nodeType == "Scene") {
		return make_unique<Scene>();
	} else if (nodeType == "Transform") {
		return make_unique<Transform>();
	} else if (nodeType == "Group") {
		return make_unique<Group>();
	} else if (nodeType == "Shape") {
		return make_unique<Shape>();
	} else if (nodeType == "Appearance") {
		return make_unique<Appearance>();
	} else if (nodeType == "ImageTexture") {
		return make_unique<ImageTexture>();
	} else if (nodeType == "TextureTransform") {
		return make_unique<TextureTransform>();
	} else if (nodeType == "Material") {
		return make_unique<Material>();
	} else if (nodeType == "IndexedFaceSet") {
		return make_unique<IndexedFaceSet>();
	} else if (nodeType == "Coordinate") {
		return make_unique<Coordinate>();
	} else if (nodeType == "Normal") {
		return make_unique<Normal>();
	} else if (nodeType == "TextureCoordinate") {
		return make_unique<TextureCoordinate>();
	} else if (nodeType == "Viewpoint") {
		return make_unique<Viewpoint>();
	} else if (nodeType == "PointLight") {
		return make_unique<PointLight>();
	} 
	return make_unique<NullNode>();
}

X3dNode::X3dNode() = default;

auto X3dNode::SetDef(string&& def) -> void {
    _def = move(def);
}

auto X3dNode::SetUse(std::string && use) -> void {
	_use = move(use);
}
    
auto X3dNode::GetDef() const -> const string& {
    return _def;
}

auto X3dNode::GetUse() const -> std::string const & {
	return _use;
}

}