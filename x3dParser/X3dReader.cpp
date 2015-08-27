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

auto inline ToPoint3(const Float3& in) -> Vector3f {
    return {in.x, in.y, in.z};
}

auto inline ToPoint2(const Float2& in) -> Vector2f {
    return {in.x, in.y};
}

auto X3dReader::Read(string const& filename) -> std::unique_ptr<core::Scene> {
	std::ifstream file{ filename };
	auto x3dNode = X3dParser::Parse(file);
	auto x3dReader = X3dReader{};
	return x3dReader.Read(*static_cast<X3d*>(x3dNode.release()));
}

auto X3dReader::Read(IndexedFaceSet & indexedFaceSet) ->  Mesh * {
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

auto X3dReader::Read(Transform & transform) -> Movable *
{
	Movable * ret = nullptr;
	auto& group = transform.GetGroup();
	auto& viewpoint = transform.GetViewpoint();
	auto& transformChildren = transform.GetTransform();
	auto& pointLight = transform.GetPointLight();
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

auto X3dReader::Read(Scene & scene) -> std::unique_ptr<core::Scene> {
	
	auto& transforms = scene.GetTransform();
	for (auto& transform : transforms) {
		_scene->Stage(Read(*transform));
	}
	return move(_scene);
}

auto X3dReader::Read(X3d & x3d) -> std::unique_ptr<core::Scene> {
	return Read(*x3d.GetScene());
}

auto X3dReader::Read(Shape & shape) -> core::Model* {
	auto ret = _scene->CreateModel();
	auto newShape = _scene->CreateShape(ret);

	newShape->SetShaderProgram(_scene->CreateDefaultShaderProgram());

	auto& appearance = *shape.GetAppearance();
	auto& imageTexture = appearance.GetImageTexture();
	if (nullptr != imageTexture) {
		auto textures = Read(*imageTexture);
		for (auto & t : textures) {
			newShape->AddTexture(t);
		}
	}
	auto& material = appearance.GetMaterial();
	if (nullptr != material) {
		auto diffuse = material->GetDiffuseColor();
		newShape->SetDiffuse({ diffuse.x, diffuse.y, diffuse.z });
		auto specular = material->GetSpecularColor();
		newShape->SetSpecular({ specular.x, specular.y, specular.z });
		auto emissive = material->GetEmissiveColor();
		newShape->SetEmissive({ emissive.x, emissive.y, emissive.z });
		newShape->SetAmbientIntensity(material->GetAmbientIntensity());
		newShape->SetShininess(material->GetShininess());
		newShape->SetTransparency(material->GetTransparency());
	}
	auto mesh = Read(*shape.GetIndexedFaceSet());
	newShape->SetMesh(mesh);

	return ret;
}

auto X3dReader::Read(ImageTexture & imageTexture) -> vector<core::Texture *> {
	auto ret = vector<core::Texture *>();
	auto urls = imageTexture.GetUrl();
	// todo: support multiple urls. for now only use second url 
	// (which is absolute path in x3d file generated from blender)
	ret.emplace_back(_scene->CreateTexture(urls[1]));
	return ret;
}

auto X3dReader::Read(Viewpoint & viewpoint) -> Camera * {
	auto ret = _scene->CreateCamera();
	ret->SetPerspective(1, 100, viewpoint.GetFieldOfView(), 1);
	return ret;
}

auto X3dReader::Read(PointLight & pointLight) -> core::PointLight * {
	auto ret = _scene->CreatePointLight();
	ret->SetAmbientIntensity(pointLight.GetAmbientIntensity());
	ret->SetColor(ToPoint3(pointLight.GetColor()));
	ret->SetIntensity(pointLight.GetIntensity());
	ret->SetLocation(ToPoint3(pointLight.GetLocation()));
	ret->SetRadius(pointLight.GetRadius());
	return ret;
}

}