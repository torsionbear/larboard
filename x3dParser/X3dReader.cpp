#include "X3dReader.h"

#include <boost/filesystem.hpp>

using std::vector;
using std::make_unique;
using std::move;
using std::unique_ptr;
using std::make_unique;
using std::string;

using core::Mesh;
using core::Vertex;
using core::SceneNode;
using core::Scene;
using core::Texture;
using core::Model;
using core::Camera;
using core::Movable;

namespace x3dParser {

auto inline ToPoint4(Float3 const& in) -> core::Point4f {
    return {in.x, in.y, in.z, 1.0f};
}

auto inline ToVector3(Float3 const& in) -> core::Vector3f {
    return{ in.x, in.y, in.z };
}

auto inline ToVector2(Float2 const& in) -> core::Vector2f {
    return {in.x, in.y};
}

auto X3dReader::Read(core::Scene * scene) -> void {
    _scene = scene;
	std::ifstream file{ _pathName.generic_string() };
	assert(file);
	auto nodes = X3dParser().Parse(file);
	ReadX3d(*static_cast<X3d*>(nodes[0].get()));
}

auto X3dReader::ReadIndexedFaceSet(IndexedFaceSet const& indexedFaceSet, core::StaticModelGroup & staticModelGroup) ->  Mesh<Vertex> * {
    if(!indexedFaceSet.GetNormalPerVertex()) {
        throw "normalPerVertex is false";
    }

    auto& coordIndex = indexedFaceSet.GetCoordIndex();
    auto& texCoordIndex = indexedFaceSet.GetTexCoordIndex();
    auto& coordinate = indexedFaceSet.GetCoordinate()->GetPoint();
    auto& normal = indexedFaceSet.GetNormal()->GetVector();
    auto& textureCoordinate = indexedFaceSet.GetTextureCoordinate()->GetPoint();

	auto vertexData = vector<Vertex>{};
	for (auto i = 0u; i < coordIndex.size(); ++i) {
		vertexData.push_back({ ToVector3(coordinate.at(coordIndex[i].a)), ToVector3(normal.at(coordIndex[i].a)), ToVector2(textureCoordinate.at(texCoordIndex[i].a))});
		vertexData.push_back({ ToVector3(coordinate.at(coordIndex[i].b)), ToVector3(normal.at(coordIndex[i].b)), ToVector2(textureCoordinate.at(texCoordIndex[i].b))});
		vertexData.push_back({ ToVector3(coordinate.at(coordIndex[i].c)), ToVector3(normal.at(coordIndex[i].c)), ToVector2(textureCoordinate.at(texCoordIndex[i].c))});
    }
	return staticModelGroup.CreateMesh(move(vertexData));;
}

auto X3dReader::ReadIndexedTriangleSet(IndexedTriangleSet const& indexedTriangleSet, core::StaticModelGroup & staticModelGroup) ->  Mesh<Vertex> * {
	if (!indexedTriangleSet.GetNormalPerVertex()) {
		throw "normalPerVertex is false";
	}

	auto index = indexedTriangleSet.GetIndex();
	auto& coordinate = indexedTriangleSet.GetCoordinate()->GetPoint();
	auto& normal = indexedTriangleSet.GetNormal()->GetVector();
	auto& textureCoordinate = indexedTriangleSet.GetTextureCoordinate()->GetPoint();

	auto vertexData = vector<Vertex>{};
	for (auto i = 0u; i < coordinate.size(); ++i) {
		vertexData.push_back({ ToVector3(coordinate[i]), ToVector3(normal[i]), ToVector2(textureCoordinate[i]) });
	}
	return staticModelGroup.CreateMesh(move(vertexData), move(index));
}

auto X3dReader::ReadTransform(Transform const& transform, core::StaticModelGroup & staticModelGroup) -> Movable *
{
	Movable * ret = nullptr;
	auto group = transform.GetGroup();
	auto viewpoint = transform.GetViewpoint();
	auto transformChildren = transform.GetTransform();
	auto pointLight = transform.GetPointLight();
	auto directionalLight = transform.GetDirectionalLight();
	auto spotLight = transform.GetSpotLight();
	if (nullptr != group) {
		auto& shapes = group->GetShape();
		ret = ReadShapes(shapes, staticModelGroup);
	} else if (nullptr != viewpoint) {
		ret = ReadViewpoint(*viewpoint);
	} else if (nullptr != pointLight) {
		ret = ReadPointLight(*pointLight);
	} else if (nullptr != directionalLight) {
		ret = ReadDirectionalLight(*directionalLight);
	} else if (nullptr != spotLight) {
		ret = ReadSpotLight(*spotLight);
	} else if (!transformChildren.empty()) {
		ret = staticModelGroup.CreateMovable();
		for (auto& transformChild : transformChildren) {
            ReadTransform(*transformChild, staticModelGroup)->AttachTo(*ret);
		}
	} else {
		ret = staticModelGroup.CreateMovable();
	}

	// Does not support scale yet.
	auto rotation = transform.GetRotation();
	ret->Rotate(rotation.x, rotation.y, rotation.z, rotation.a);

	auto translation = transform.GetTranslation();
	ret->Translate(translation.x, translation.y, translation.z);

	return ret;
}

auto X3dReader::ReadScene(Scene const& scene) -> void {
	auto& transforms = scene.GetTransform();
	for (auto& transform : transforms) {
		_scene->Stage(ReadTransform(*transform, _scene->GetStaticModelGroup()));
	}
}

auto X3dReader::ReadX3d(X3d const& x3d) -> void {
	ReadScene(*x3d.GetScene());
}

auto X3dReader::ReadShapes(vector<Shape*> const& shapes, core::StaticModelGroup & staticModelGroup) -> core::Model* {
	auto ret = staticModelGroup.CreateModel();
	for (auto const& shape : shapes) {
		auto newShape = staticModelGroup.CreateShape(ret);

		auto appearanceNode = shape->GetAppearance();
		auto imageTextureNode = appearanceNode->GetImageTexture();
		if (nullptr != imageTextureNode) {
            auto textures = ReadImageTexture(*imageTextureNode, staticModelGroup);
            for (auto texture : textures) {
                newShape->AddTexture(texture);
            }
            if (staticModelGroup.GetShaderProgram("textured") == nullptr) {
                staticModelGroup.CreateShaderProgram("textured", "shader/textured_v.shader", "shader/textured_f.shader");
            }
            newShape->SetShaderProgram(staticModelGroup.GetShaderProgram("textured"));
        } else {
            if (staticModelGroup.GetShaderProgram("untextured") == nullptr) {
                staticModelGroup.CreateShaderProgram("untextured", "shader/untextured_v.shader", "shader/untextured_f.shader");
            }
            newShape->SetShaderProgram(staticModelGroup.GetShaderProgram("untextured"));
        }
        auto materialNode = appearanceNode->GetMaterial();
        if (nullptr != materialNode) {
            auto material = ReadMaterial(*materialNode, staticModelGroup);
            for (auto & texture : newShape->GetTextures()) {
                switch (texture->GetType()) {
                case core::TextureUsage::DiffuseMap:
                    material->_hasDiffuseMap = true;
                    break;
                case core::TextureUsage::NormalMap:
                    material->_hasNormalMap = true;
                    break;
                case core::TextureUsage::SpecularMap:
                    material->_hasSpecularMap = true;
                    break;
                case core::TextureUsage::EmissiveMap:
                    material->_hasEmissiveMap = true;
                    break;
                }
            }
            newShape->SetMaterial(material);
        }
		auto indexedTriangleSet = shape->GetIndexedTriangleSet();
		if (indexedTriangleSet != nullptr) {
			newShape->SetMesh(ReadIndexedTriangleSet(*indexedTriangleSet, staticModelGroup));
		} else {
			auto mesh = ReadIndexedFaceSet(*shape->GetIndexedFaceSet(), staticModelGroup);
			newShape->SetMesh(mesh);
		}
	}

	return ret;
}

auto X3dReader::ReadMaterial(Material const & material, core::StaticModelGroup & staticModelGroup) -> core::Material * {
	auto use = material.GetUse();
	if (!use.empty()) {
        // hack here: always make a new copy of material in case of USE.
        // core::Material contains texture information, which may be altered later,
        // so the 2 same x3dParser::Material nodes might become 2 different core::Material instance.
        auto copy = make_unique<core::Material>(*_materials[use]);
        return staticModelGroup.AddMaterial(std::move(copy));
	}
	auto ret = staticModelGroup.CreateMaterial();
	auto diffuse = material.GetDiffuseColor();
	ret->SetDiffuse(ToVector3(diffuse));
	auto specular = material.GetSpecularColor();
	ret->SetSpecular(ToVector3(specular));
	auto emissive = material.GetEmissiveColor();
	ret->SetEmissive(ToVector3(material.GetEmissiveColor()));
	ret->SetShininess(material.GetShininess());
	ret->SetTransparency(material.GetTransparency());

    auto materialName = material.GetDef();
    if (!materialName.empty()) {
        _materials[materialName] = ret;
    }
	return ret;
}

auto X3dReader::ReadImageTexture(ImageTexture const& imageTexture, core::StaticModelGroup & staticModelGroup) -> vector<Texture *> {
    auto ret = vector<Texture *>{};
	auto use = imageTexture.GetUse();
	if (!use.empty()) {
        return _imageTextures[use];
    }
    auto urls = imageTexture.GetUrl();
	// todo: support multiple urls. for now only use first url 
	// which is relative path in x3d file generated from blender
    auto url = urls[0];
    auto dotPos = url.find_last_of('.');
    auto urlExt = url.substr(dotPos);
    auto urlWithoutExt = url.substr(0, dotPos);
    auto folder = _pathName.parent_path();

    auto diffuseMapFilename = folder / (urlWithoutExt + urlExt);
    if (boost::filesystem::exists(diffuseMapFilename)) {
        ret.push_back(staticModelGroup.CreateTexture(diffuseMapFilename.generic_string(), core::TextureUsage::DiffuseMap));
    }
    auto normalMapFilename = folder / (urlWithoutExt + "_normal" + urlExt);
    if (boost::filesystem::exists(normalMapFilename)) {
        ret.push_back(staticModelGroup.CreateTexture(normalMapFilename.generic_string(), core::TextureUsage::NormalMap));
    }
    auto specularMapFilename = folder / (urlWithoutExt + "_specular" + urlExt);
    if (boost::filesystem::exists(specularMapFilename)) {
        ret.push_back(staticModelGroup.CreateTexture(specularMapFilename.generic_string(), core::TextureUsage::SpecularMap));
    }
    auto emissiveMapFilename = folder / (urlWithoutExt + "_emissive" + urlExt);
    if (boost::filesystem::exists(emissiveMapFilename)) {
        ret.push_back(staticModelGroup.CreateTexture(emissiveMapFilename.generic_string(), core::TextureUsage::EmissiveMap));
    }
    auto textureName = imageTexture.GetDef();
    if (!textureName.empty()) {
        _imageTextures[textureName] = ret;
    }
    return ret;
}

auto X3dReader::ReadViewpoint(Viewpoint const& viewpoint) -> Camera * {
	auto ret = _scene->CreateCamera();
	ret->SetPerspective(1, viewpoint.GetFieldOfView(), 0.1f, 1000.0f);
	return ret;
}

auto X3dReader::ReadPointLight(PointLight const& pointLight) -> core::PointLight * {
	auto ret = _scene->CreatePointLight();
	// ignore ambientIntensity. Use standalone ambient light instead
	//ret->SetAmbientIntensity(pointLight.GetAmbientIntensity());
	auto color = pointLight.GetColor();
	auto intensity = pointLight.GetIntensity();
	ret->SetColor(core::Vector4f{ color.x, color.y, color.z, 1.0f } * intensity);
	ret->Translate(ToPoint4(pointLight.GetLocation()));
	ret->SetAttenuation(ToVector3(pointLight.GetAttenuation()));
	ret->SetRadius(pointLight.GetRadius());
	return ret;
}

auto X3dReader::ReadDirectionalLight(DirectionalLight const & directionalLight) -> core::DirectionalLight * {
	auto ret = _scene->CreateDirectionalLight();
	// ignore ambientIntensity. Use standalone ambient light instead
	ret->SetColor(ToVector3(directionalLight.GetColor()) * directionalLight.GetIntensity());
	auto direction = directionalLight.GetDirection();
	ret->SetDirection(core::Vector4f{direction.x, direction.y, direction.z, 0.0f});
	return ret;
}

auto X3dReader::ReadSpotLight(SpotLight const & spotLight) -> core::SpotLight * {
	auto ret = _scene->CreateSpotLight();
	// ignore ambientIntensity. Use standalone ambient light instead
	auto color = spotLight.GetColor();
	auto intensity = spotLight.GetIntensity();
	ret->SetColor(core::Vector4f{ color.x, color.y, color.z, 1.0f } *intensity);
	ret->Translate(ToPoint4(spotLight.GetLocation()));
	ret->SetAttenuation(ToVector3(spotLight.GetAttenuation()));
	ret->SetRadius(spotLight.GetRadius());
	auto direction = spotLight.GetDirection();
	ret->SetDirection(core::Vector4f{ direction.x, direction.y, direction.z, 0.0f });
	ret->SetBeamWidth(spotLight.GetBeamWidth());
	ret->SetCutOffAngle(spotLight.GetCutOffAngle());
	return ret;
}

}