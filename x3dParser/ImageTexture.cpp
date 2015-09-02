#include "ImageTexture.h"

#include <boost/algorithm/string.hpp>

using std::string;

namespace x3dParser {

auto ImageTexture::SetAttribute(string const& attribute, string&& value) -> void {
    if(attribute.compare("url") == 0) {
        SetUrl(move(value));
    } else if (attribute.compare("DEF") == 0) {
        SetDef(move(value));
    } else if (attribute.compare("USE") == 0) {
		SetUse(move(value));
	}
}
    
auto ImageTexture::AddChild(X3dNode *) -> void {
}
    
auto ImageTexture::GetUrl() const -> std::vector<std::string> {
    return _url;
}

auto ImageTexture::SetUrl(string&& url) -> void {
    // url is defined as <xs:list itemType="xs:string"/>, see http://www.web3d.org/specifications/X3dSchemaDocumentation3.3/x3d-3.3_MFString.html#Link7E
    // xs:list elements are always separated by whitespace, regardless of quotation mark. See http://www.w3.org/TR/xmlschema11-2/#atomic-vs-list 2.4.1.2
    // so we consider no whitespace in each url element, hence we can straightly spilt elements by whitespace.
    boost::algorithm::split(_url, url, boost::algorithm::is_any_of(" \r\n\t"), boost::algorithm::token_compress_on);
    //remove double quote
    for(auto& u : _url) {
        u = u.substr(1u, u.size() - 2u);
    }
}

}