#include "X3dReader.h"

#include "X3dParser.h"

using std::vector;
using std::make_unique;
using std::move;
using std::unique_ptr;
using std::make_unique;
using std::string;

using core::Vector2f;
using core::Vector3f;
using core::Mesh;
using core::Vertex;
using core::SceneNode;
using core::Scene;
using core::Texture;
using core::Model;
using core::Camera;
using core::Movable;

namespace x3dParser {

auto inline ToPoint3(Float3 const& in) -> Vector3f {
    return {in.x, in.y, in.z};
}

auto inline ToPoint2(Float2 const& in) -> Vector2f {
    return {in.x, in.y};
}

auto X3dReader::Read(string const& filename) -> std::unique_ptr<core::Scene> {
	std::ifstream file{ filename };
	auto x3dParser = X3dParser();
	auto nodes = x3dParser.Parse(file);
	auto x3dReader = X3dReader{};
	return x3dReader.Read(*static_cast<X3d*>(nodes[0].get()));
}

auto X3dReader::Read(IndexedFaceSet const& indexedFaceSet) ->  Mesh * {
    if(!indexedFaceSet.GetNormalPerVertex()) {
        throw "normalPerVertex is false";
    }

	auto ret = _scene->CreateMesh();
    auto& coordIndex = indexedFaceSet.GetCoordIndex();
    auto& texCoordIndex = indexedFaceSet.GetTexCoordIndex();
    auto& coordinate = indexedFaceSet.GetCoordinate()->GetPoint();
    auto& normal = indexedFaceSet.GetNormal()->GetVector();
    auto& textureCoordinate = indexedFaceSet.GetTextureCoordinate()->GetPoint();

	auto vertexData = vector<Vertex>{};
	for (auto i = 0u; i < coordIndex.size(); ++i) {
		vertexData.push_back({ ToPoint3(coordinate.at(coordIndex[i].a)), ToPoint3(normal.at(coordIndex[i].a)), ToPoint2(textureCoordinate.at(texCoordIndex[i].a))});
		vertexData.push_back({ ToPoint3(coordinate.at(coordIndex[i].b)), ToPoint3(normal.at(coordIndex[i].b)), ToPoint2(textureCoordinate.at(texCoordIndex[i].b))});
		vertexData.push_back({ ToPoint3(coordinate.at(coordIndex[i].c)), ToPoint3(normal.at(coordIndex[i].c)), ToPoint2(textureCoordinate.at(texCoordIndex[i].c))});
    }
	ret->SetVertexData(move(vertexData));

	return ret;
}

auto X3dReader::Read(Transform const& transform) -> Movable *
{
	Movable * ret = nullptr;
	auto group = transform.GetGroup();
	auto viewpoint = transform.GetViewpoint();
	auto transformChildren = transform.GetTransform();
	auto pointLight = transform.GetPointLight();
	if (nullptr != group) {
		auto& shapes = group->GetShape();
		assert(1u == shapes.size());	// support only 1 shape under a transform. Multiple shapes share the same transform does not make sense.
		ret = Read(*shapes[0]);
	} else if (nullptr != viewpoint) {
		ret = Read(*viewpoint);
	} else if (nullptr != pointLight) {
		ret = Read(*pointLight);
	} else if (!transformChildren.empty()) {
		ret = _scene->CreateMovable();
		for (auto& transformChild : transformChildren) {
			Read(*transformChild)->AttachTo(*ret);
		}
	} else {
		ret = _scene->CreateMovable();
	}

	// Does not support scale yet.
	auto rotation = transform.GetRotation();
	ret->Rotate(rotation.x, rotation.y, rotation.z, rotation.a);

	auto translation = transform.GetTranslation();
	ret->Translate(translation.x, translation.y, translation.z);

	return ret;
}

auto X3dReader::Read(Scene const& scene) -> std::unique_ptr<core::Scene> {
	
	auto& transforms = scene.GetTransform();
	for (auto& transform : transforms) {
		_scene->Stage(Read(*transform));
	}
	return move(_scene);
}

auto X3dReader::Read(X3d const& x3d) -> std::unique_ptr<core::Scene> {
	return Read(*x3d.GetScene());
}

auto X3dReader::Read(Shape const& shape) -> core::Model* {
	auto ret = _scene->CreateModel();
	auto newShape = _scene->CreateShape(ret);

	newShape->SetShaderProgram(_scene->GetDefaultShaderProgram());

	auto appearance = shape.GetAppearance();
	auto imageTexture = appearance->GetImageTexture();
	if (nullptr != imageTexture) {
		newShape->AddTexture(Read(*imageTexture));
	}
	auto material = appearance->GetMaterial();
	if (nullptr != material) {
		newShape->SetMaterial(Read(*material));
	}
	auto mesh = Read(*shape.GetIndexedFaceSet());
	newShape->SetMesh(mesh);

	return ret;
}

auto X3dReader::Read(Material const & material) -> core::Material * {
	auto use = material.GetUse();
	if (!use.empty()) {
		return _scene->GetMaterial(use);
	}
	auto materialName = material.GetDef();
	auto ret = _scene->CreateMaterial(materialName);

	auto diffuse = material.GetDiffuseColor();
	ret->SetDiffuse({ diffuse.x, diffuse.y, diffuse.z, 1.0f });
	auto specular = material.GetSpecularColor();
	ret->SetSpecular({ specular.x, specular.y, specular.z, 1.0f });
	auto emissive = material.GetEmissiveColor();
	ret->SetEmissive({ emissive.x, emissive.y, emissive.z, 1.0f });
	ret->SetAmbientIntensity(material.GetAmbientIntensity());
	ret->SetShininess(material.GetShininess());
	ret->SetTransparency(material.GetTransparency());

	return ret;
}

auto X3dReader::Read(ImageTexture const& imageTexture) -> core::Texture * {
	auto use = imageTexture.GetUse();
	if (!use.empty()) {
		return _scene->GetTexture(use);
	}
	auto textureName = imageTexture.GetDef();
	auto urls = imageTexture.GetUrl();
	// todo: support multiple urls. for now only use second url 
	// (which is absolute path in x3d file generated from blender)
	return _scene->CreateTexture(textureName, urls[1]);
}

auto X3dReader::Read(Viewpoint const& viewpoint) -> Camera * {
	auto ret = _scene->CreateCamera();
	ret->SetPerspective(1, 100, viewpoint.GetFieldOfView(), 1);
	return ret;
}

auto X3dReader::Read(PointLight const& pointLight) -> core::PointLight * {
	auto ret = _scene->CreatePointLight();
	ret->SetAmbientIntensity(pointLight.GetAmbientIntensity());
	ret->SetColor(ToPoint3(pointLight.GetColor()));
	ret->SetIntensity(pointLight.GetIntensity());
	ret->SetLocation(ToPoint3(pointLight.GetLocation()));
	ret->SetRadius(pointLight.GetRadius());
	return ret;
}

}