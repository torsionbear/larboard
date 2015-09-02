#include <fstream>
#include <sstream>
#include <iostream>
#include <numeric>

#include <gtest/gtest.h>

#include "x3dParser/X3dParser.h"
#include "x3dParser/x3dReader.h"
#include "x3dParser/X3d.h"
#include "x3dParser/Transform.h"
#include "x3dParser/Group.h"
#include "x3dParser/Shape.h"
#include "x3dParser/Appearance.h"
#include "x3dParser/ImageTexture.h"
#include "x3dParser/TextureTransform.h"
#include "x3dParser/Material.h"
#include "x3dParser/IndexedFaceSet.h"
#include "x3dParser/Coordinate.h"
#include "x3dParser/Normal.h"
#include "x3dParser/TextureCoordinate.h"
#include "x3dParser/Viewpoint.h"

using std::ifstream;
using std::vector;

using namespace x3dParser;

class X3dParserTest : public ::testing::Test {
public:
protected:
	X3dParser _sut;
};

//TEST_F(X3dParserTest, read) {
//	auto result = X3dReader::Read("D:/torsionbear/working/larboard/larboard/x3dParserMicroTest/square.x3d");
//}

TEST_F(X3dParserTest, parse) {
	ifstream x3dFile{ "D:/torsionbear/working/larboard/larboard/x3dParserMicroTest/square.x3d" };
	auto result = _sut.Parse(x3dFile);
	auto x3d = result.front().get();

	ASSERT_EQ(typeid(X3d), typeid(*x3d));

	auto const& scene = static_cast<X3d*>(x3d)->GetScene();
	ASSERT_EQ(typeid(Scene), typeid(*scene));

	auto& transform0 = *scene->GetTransform()[0];
	ASSERT_EQ(typeid(Transform), typeid(transform0));
	ASSERT_EQ("Plane_TRANSFORM", transform0.GetDef());
	ASSERT_EQ(Float3(0, 0, 0), transform0.GetTranslation());
	ASSERT_EQ(Float3(1, 1, 1), transform0.GetScale());
	ASSERT_EQ(Float4(1.0f, 0.0f, 0.0f, 0.0f), transform0.GetRotation());

	auto& transform01 = *transform0.GetTransform()[0];
	ASSERT_EQ(typeid(Transform), typeid(transform01));
	ASSERT_EQ("Plane_ifs_TRANSFORM", transform01.GetDef());
	ASSERT_EQ(Float3(0, 0, 0), transform01.GetTranslation());
	ASSERT_EQ(Float3(1, 1, 1), transform01.GetScale());
	ASSERT_EQ(Float4(1, 0, 0, 0), transform01.GetRotation());

	auto group = transform01.GetGroup();
	ASSERT_EQ(typeid(Group), typeid(*group));
	ASSERT_EQ("group_ME_Plane", group->GetDef());

	auto& shape = *group->GetShape()[0];
	ASSERT_EQ(typeid(Shape), typeid(shape));

	auto appearance = shape.GetAppearance();
	ASSERT_EQ(typeid(Appearance), typeid(*appearance));

	auto imageTexture = appearance->GetImageTexture();
	ASSERT_EQ(typeid(ImageTexture), typeid(*imageTexture));
	ASSERT_EQ("IM_Pedobear_png", imageTexture->GetDef());
	auto urls = imageTexture->GetUrl();
	ASSERT_EQ(2u, urls.size());
	ASSERT_EQ("Pedobear.png", urls[0]);
	ASSERT_EQ("D:/torsionbear/working/larboard/Modeling/square/Pedobear.png", urls[1]);

	auto textureTransform = appearance->GetTextureTransform();
	ASSERT_EQ(typeid(TextureTransform), typeid(*textureTransform));
	ASSERT_EQ(Float2(0, 0), textureTransform->GetTranslation());
	ASSERT_EQ(Float2(1, 1), textureTransform->GetScale());
	ASSERT_EQ(0, textureTransform->GetRotation());

	auto material = appearance->GetMaterial();
	ASSERT_EQ(typeid(Material), typeid(*material));
	ASSERT_EQ("MA_Material_001", material->GetDef());
	ASSERT_EQ(Float3(.8f, .8f, .8f), material->GetDiffuseColor());
	ASSERT_EQ(Float3(.401f, .401f, .401f), material->GetSpecularColor());
	ASSERT_EQ(Float3(.0f, .0f, .0f), material->GetEmissiveColor());
	ASSERT_EQ(0.333f, material->GetAmbientIntensity());
	ASSERT_EQ(0.098f, material->GetShininess());
	ASSERT_EQ(0.0f, material->GetTransparency());

	auto indexedFaceSet = shape.GetIndexedFaceSet();
	ASSERT_EQ(typeid(IndexedFaceSet), typeid(*indexedFaceSet));
	ASSERT_TRUE(indexedFaceSet->GetSolid());
	ASSERT_TRUE(indexedFaceSet->GetNormalPerVertex());
	ASSERT_EQ(vector<ULong3>({ { 0, 1, 2 },{ 3, 4, 5 } }), indexedFaceSet->GetTexCoordIndex());
	ASSERT_EQ(vector<ULong3>({ { 1, 3, 2 },{ 0, 1, 2 } }), indexedFaceSet->GetCoordIndex());

	auto coordinate = indexedFaceSet->GetCoordinate();
	ASSERT_EQ(typeid(Coordinate), typeid(*coordinate));
	ASSERT_EQ("coords_ME_Plane", coordinate->GetDef());
	ASSERT_EQ(vector<Float3>({ { -1, -1, 0 },{ 1, -1, 0 },{ -1, 1, 0 },{ 1, 1, 0 } }), coordinate->GetPoint());

	auto normal = indexedFaceSet->GetNormal();
	ASSERT_EQ(typeid(Normal), typeid(*normal));
	ASSERT_EQ("normals_ME_Plane", normal->GetDef());
	ASSERT_EQ(vector<Float3>({ { 0, 0, 1 },{ 0, 0, 1 },{ 0, 0, 1 },{ 0, 0, 1 } }), normal->GetVector());

	auto textureCoordinate = indexedFaceSet->GetTextureCoordinate();
	ASSERT_EQ(typeid(TextureCoordinate), typeid(*textureCoordinate));
	ASSERT_EQ(vector<Float2>({ { 1, 0 },{ 1, 1 },{ 0, 1 },{ 0, 0 },{ 1, 0 },{ 0, 1 } }), textureCoordinate->GetPoint());

	auto& transform1 = *scene->GetTransform()[1];
	ASSERT_EQ(typeid(Transform), typeid(transform1));
	ASSERT_EQ("Lamp_TRANSFORM", transform1.GetDef());
	ASSERT_EQ(Float3(4.076245f, 1.005454f, 5.903862f), transform1.GetTranslation());
	ASSERT_EQ(Float3(1.0f, 1.0f, 1.0f), transform1.GetScale());
	ASSERT_EQ(Float4(0.205942f, 0.331517f, 0.920698f, 1.926274f), transform1.GetRotation());

	auto pointLight = transform1.GetPointLight();
	ASSERT_EQ(typeid(PointLight), typeid(*pointLight));
	ASSERT_EQ("LA_Lamp", pointLight->GetDef());
	ASSERT_DOUBLE_EQ(0.0000f, pointLight->GetAmbientIntensity());
	ASSERT_EQ(Float3(1.0f, 1.0f, 1.0f), pointLight->GetColor());
	ASSERT_DOUBLE_EQ(0.5714f, pointLight->GetIntensity());
	ASSERT_DOUBLE_EQ(30.0f, pointLight->GetRadius());
	ASSERT_EQ(Float3(0.0f, 0.0f, 0.0f), pointLight->GetLocation());


	auto& transform2 = *scene->GetTransform()[2];
	ASSERT_EQ(typeid(Transform), typeid(transform2));
	ASSERT_EQ("Camera_TRANSFORM", transform2.GetDef());
	ASSERT_EQ(Float3(1.0f, -1.0f, 2.0f), transform2.GetTranslation());
	ASSERT_EQ(Float3(1.0f, 1.0f, 1.0f), transform2.GetScale());
	ASSERT_EQ(Float4(0.678598f, 0.281084f, 0.678599f, 1.096056f), transform2.GetRotation());

	auto viewpoint = transform2.GetViewpoint();
	ASSERT_EQ(typeid(Viewpoint), typeid(*viewpoint));
	ASSERT_EQ("CA_Camera", viewpoint->GetDef());
	ASSERT_EQ(Float3(0, 0, 0), viewpoint->GetCenterOfRotation());
	ASSERT_EQ(Float3(0, 0, 0), viewpoint->GetPosition());
	ASSERT_EQ(Float4(0.56f, - 0.40f, -0.73f, 0.00f), viewpoint->GetOrientation());
	ASSERT_TRUE(equal(viewpoint->GetFieldOfView(), 0.858f));
}