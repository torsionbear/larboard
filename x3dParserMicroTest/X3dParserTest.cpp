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
#include "x3dParser/IndexedTriangleSet.h"
#include "x3dParser/Coordinate.h"
#include "x3dParser/Normal.h"
#include "x3dParser/TextureCoordinate.h"
#include "x3dParser/Viewpoint.h"

using std::ifstream;
using std::vector;
using std::unique_ptr;

using namespace x3dParser;

class X3dParserTest : public ::testing::Test {
public:
    virtual auto SetUp() -> void {
        _square = X3dParser().Parse(ifstream{ "D:/torsionbear/working/larboard/larboard/x3dParserMicroTest/square.x3d" });
    }
protected:
    vector<unique_ptr<X3dNode>> _square;
};

TEST_F(X3dParserTest, indexedTriangleSet) {
    auto square_triangulated = X3dParser().Parse(ifstream{ "D:/torsionbear/working/larboard/larboard/x3dParserMicroTest/square_trianglated.x3d" });

    auto const& scene = static_cast<X3d*>(square_triangulated.front().get())->GetScene();
    auto& planeTransform = *scene->GetTransform()[2];
    auto& planIfsTransform = *planeTransform.GetTransform()[0];
    auto groupMePlane = planIfsTransform.GetGroup();
    auto& shape = *groupMePlane->GetShape()[0];

    auto indexedTriangleSet = shape.GetIndexedTriangleSet();
    ASSERT_EQ(typeid(IndexedTriangleSet), typeid(*indexedTriangleSet));
    ASSERT_TRUE(indexedTriangleSet->GetSolid());
    ASSERT_TRUE(indexedTriangleSet->GetNormalPerVertex());
    ASSERT_EQ(vector<unsigned int>({ 0, 1, 2, 3, 0, 2 }), indexedTriangleSet->GetIndex());

    auto coordinate = indexedTriangleSet->GetCoordinate();
    ASSERT_EQ(typeid(Coordinate), typeid(*coordinate));
    ASSERT_EQ(vector<Float3>({ { 1, -1, 0 },{ 1, 1, 0 },{ -1, 1, 0 },{ -1, -1, 0 } }), coordinate->GetPoint());

    auto normal = indexedTriangleSet->GetNormal();
    ASSERT_EQ(typeid(Normal), typeid(*normal));
    ASSERT_EQ(vector<Float3>({ { 0, 0, 1 },{ 0, 0, 1 },{ 0, 0, 1 },{ 0, 0, 1 } }), normal->GetVector());

    auto textureCoordinate = indexedTriangleSet->GetTextureCoordinate();
    ASSERT_EQ(typeid(TextureCoordinate), typeid(*textureCoordinate));
    ASSERT_EQ(vector<Float2>({ { 1, 0 },{ 1, 1 },{ 0, 1 },{ 0, 0 } }), textureCoordinate->GetPoint());
}

TEST_F(X3dParserTest, x3d_scene) {
    auto x3d = _square.front().get();
    ASSERT_EQ(typeid(X3d), typeid(*x3d));

    auto const& scene = static_cast<X3d*>(x3d)->GetScene();
    ASSERT_EQ(typeid(Scene), typeid(*scene));
}

TEST_F(X3dParserTest, directionalLight) {
    auto const& scene = static_cast<X3d*>(_square.front().get())->GetScene();
    auto& sunTransform = *scene->GetTransform()[0];
    auto& directionalLight = *sunTransform.GetDirectionalLight();

    ASSERT_EQ(typeid(DirectionalLight), typeid(directionalLight));
    ASSERT_FLOAT_EQ(0.0f, directionalLight.GetAmbientIntensity());
    ASSERT_EQ(Float3(1.0f, 1.0f, 1.0f), directionalLight.GetColor());
    ASSERT_FLOAT_EQ(0.5714f, directionalLight.GetIntensity());
    ASSERT_EQ(Float3(0.0f, 0.0f, -1.0f), directionalLight.GetDirection());
}

TEST_F(X3dParserTest, spotLight) {
    auto const& scene = static_cast<X3d*>(_square.front().get())->GetScene();
    auto& spotTransform = *scene->GetTransform()[1];
    auto& spotLight = *spotTransform.GetSpotLight();

    ASSERT_EQ(typeid(SpotLight), typeid(spotLight));
    ASSERT_FLOAT_EQ(23.9518f, spotLight.GetRadius());
    ASSERT_FLOAT_EQ(0.0f, spotLight.GetAmbientIntensity());
    ASSERT_FLOAT_EQ(0.5714f, spotLight.GetIntensity());
    ASSERT_EQ(Float3(1.0f, 1.0f, 1.0f), spotLight.GetColor());
    ASSERT_FLOAT_EQ(0.2906f, spotLight.GetBeamWidth());
    ASSERT_FLOAT_EQ(0.3778f, spotLight.GetCutOffAngle());
    ASSERT_EQ(Float3(0.0f, 0.0f, -1.0f), spotLight.GetDirection());
    ASSERT_EQ(Float3(0.0f, 0.0f, 0.0f), spotLight.GetLocation());
}

TEST_F(X3dParserTest, pointLight) {
    auto const& scene = static_cast<X3d*>(_square.front().get())->GetScene();
    auto& lampTransform = *scene->GetTransform()[3];
    auto pointLight = lampTransform.GetPointLight();

    ASSERT_EQ(typeid(PointLight), typeid(*pointLight));
    ASSERT_EQ("LA_Lamp", pointLight->GetDef());
    ASSERT_DOUBLE_EQ(0.0000f, pointLight->GetAmbientIntensity());
    ASSERT_EQ(Float3(1.0f, 1.0f, 1.0f), pointLight->GetColor());
    ASSERT_DOUBLE_EQ(0.5714f, pointLight->GetIntensity());
    ASSERT_DOUBLE_EQ(30.0f, pointLight->GetRadius());
    ASSERT_EQ(Float3(0.0f, 0.0f, 0.0f), pointLight->GetLocation());
}

TEST_F(X3dParserTest, camera) {
    auto const& scene = static_cast<X3d*>(_square.front().get())->GetScene();
    auto& cameraTransform = *scene->GetTransform()[4];
    auto viewpoint = cameraTransform.GetViewpoint();

    ASSERT_EQ(typeid(Viewpoint), typeid(*viewpoint));
    ASSERT_EQ("CA_Camera", viewpoint->GetDef());
    ASSERT_EQ(Float3(0, 0, 0), viewpoint->GetCenterOfRotation());
    ASSERT_EQ(Float3(0, 0, 0), viewpoint->GetPosition());
    ASSERT_EQ(Float4(0.56f, -0.40f, -0.73f, 0.00f), viewpoint->GetOrientation());
    ASSERT_TRUE(equal(viewpoint->GetFieldOfView(), 0.858f));
}

TEST_F(X3dParserTest, group_shape_appearance) {
    auto const& scene = static_cast<X3d*>(_square.front().get())->GetScene();
    auto& planeTransform = *scene->GetTransform()[2];
    auto& planIfsTransform = *planeTransform.GetTransform()[0];

    auto groupMePlane = planIfsTransform.GetGroup();
    ASSERT_EQ(typeid(Group), typeid(*groupMePlane));
    ASSERT_EQ("group_ME_Plane", groupMePlane->GetDef());

    auto& shape = *groupMePlane->GetShape()[0];
    ASSERT_EQ(typeid(Shape), typeid(shape));

    auto appearance = shape.GetAppearance();
    ASSERT_EQ(typeid(Appearance), typeid(*appearance));
}

TEST_F(X3dParserTest, imageTexture) {
    auto const& scene = static_cast<X3d*>(_square.front().get())->GetScene();
    auto& planeTransform = *scene->GetTransform()[2];
    auto& planIfsTransform = *planeTransform.GetTransform()[0];
    auto groupMePlane = planIfsTransform.GetGroup();
    auto& shape = *groupMePlane->GetShape()[0];
    auto appearance = shape.GetAppearance();

    auto imageTexture = appearance->GetImageTexture();
    ASSERT_EQ(typeid(ImageTexture), typeid(*imageTexture));
    ASSERT_EQ("IM_Pedobear_png", imageTexture->GetDef());
    auto urls = imageTexture->GetUrl();
    ASSERT_EQ(2u, urls.size());
    ASSERT_EQ("Pedobear.png", urls[0]);
    ASSERT_EQ("D:/torsionbear/working/larboard/Modeling/square/Pedobear.png", urls[1]);
}

TEST_F(X3dParserTest, textureTransform) {
    auto const& scene = static_cast<X3d*>(_square.front().get())->GetScene();
    auto& planeTransform = *scene->GetTransform()[2];
    auto& planIfsTransform = *planeTransform.GetTransform()[0];
    auto groupMePlane = planIfsTransform.GetGroup();
    auto& shape = *groupMePlane->GetShape()[0];
    auto appearance = shape.GetAppearance();

    auto textureTransform = appearance->GetTextureTransform();
    ASSERT_EQ(typeid(TextureTransform), typeid(*textureTransform));
    ASSERT_EQ(Float2(0, 0), textureTransform->GetTranslation());
    ASSERT_EQ(Float2(1, 1), textureTransform->GetScale());
    ASSERT_EQ(0, textureTransform->GetRotation());
}

TEST_F(X3dParserTest, material) {
    auto const& scene = static_cast<X3d*>(_square.front().get())->GetScene();
    auto& planeTransform = *scene->GetTransform()[2];
    auto& planIfsTransform = *planeTransform.GetTransform()[0];
    auto groupMePlane = planIfsTransform.GetGroup();
    auto& shape = *groupMePlane->GetShape()[0];
    auto appearance = shape.GetAppearance();

    auto material = appearance->GetMaterial();
    ASSERT_EQ(typeid(Material), typeid(*material));
    ASSERT_EQ("MA_Material_001", material->GetDef());
    ASSERT_EQ(Float3(.8f, .8f, .8f), material->GetDiffuseColor());
    ASSERT_EQ(Float3(.401f, .401f, .401f), material->GetSpecularColor());
    ASSERT_EQ(Float3(.0f, .0f, .0f), material->GetEmissiveColor());
    ASSERT_EQ(0.333f, material->GetAmbientIntensity());
    ASSERT_EQ(0.098f, material->GetShininess());
    ASSERT_EQ(0.0f, material->GetTransparency());
}

TEST_F(X3dParserTest, indexedFaceSet) {
    auto const& scene = static_cast<X3d*>(_square.front().get())->GetScene();
    auto& planeTransform = *scene->GetTransform()[2];
    auto& planIfsTransform = *planeTransform.GetTransform()[0];
    auto groupMePlane = planIfsTransform.GetGroup();
    auto& shape = *groupMePlane->GetShape()[0];

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
}

TEST_F(X3dParserTest, transform) {
	auto const& scene = static_cast<X3d*>(_square.front().get())->GetScene();

    auto& sunTransform = *scene->GetTransform()[0];
    ASSERT_EQ(typeid(Transform), typeid(sunTransform));
    ASSERT_EQ("Sun_TRANSFORM", sunTransform.GetDef());
    ASSERT_EQ(Float3(1.443861f, 0.400072f, 4.394063f), sunTransform.GetTranslation());
    ASSERT_EQ(Float3(1.0f, 1.0f, 1.0f), sunTransform.GetScale());
    ASSERT_EQ(Float4(1.000000f, 0.000000f, 0.000000f, 0.314159f), sunTransform.GetRotation());

    auto& spotTransform = *scene->GetTransform()[1];
    ASSERT_EQ(typeid(Transform), typeid(spotTransform));
    ASSERT_EQ("Spot_TRANSFORM", spotTransform.GetDef());
    ASSERT_EQ(Float3(-0.431399f, 0.616424f, 2.701990f), spotTransform.GetTranslation());
    ASSERT_EQ(Float3(1.0f, 1.0f, 1.0f), spotTransform.GetScale());
    ASSERT_EQ(Float4(1.0f, 0.0f, 0.0f, 0.0f), spotTransform.GetRotation());

	auto& planeTransform = *scene->GetTransform()[2];
	ASSERT_EQ(typeid(Transform), typeid(planeTransform));
	ASSERT_EQ("Plane_TRANSFORM", planeTransform.GetDef());
	ASSERT_EQ(Float3(0, 0, 0), planeTransform.GetTranslation());
	ASSERT_EQ(Float3(1, 1, 1), planeTransform.GetScale());
	ASSERT_EQ(Float4(1.0f, 0.0f, 0.0f, 0.0f), planeTransform.GetRotation());

	auto& planIfsTransform = *planeTransform.GetTransform()[0];
	ASSERT_EQ(typeid(Transform), typeid(planIfsTransform));
	ASSERT_EQ("Plane_ifs_TRANSFORM", planIfsTransform.GetDef());
	ASSERT_EQ(Float3(0, 0, 0), planIfsTransform.GetTranslation());
	ASSERT_EQ(Float3(1, 1, 1), planIfsTransform.GetScale());
	ASSERT_EQ(Float4(1, 0, 0, 0), planIfsTransform.GetRotation());

	auto& lampTransform = *scene->GetTransform()[3];
	ASSERT_EQ(typeid(Transform), typeid(lampTransform));
	ASSERT_EQ("Lamp_TRANSFORM", lampTransform.GetDef());
	ASSERT_EQ(Float3(4.076245f, 1.005454f, 5.903862f), lampTransform.GetTranslation());
	ASSERT_EQ(Float3(1.0f, 1.0f, 1.0f), lampTransform.GetScale());
	ASSERT_EQ(Float4(0.205942f, 0.331517f, 0.920698f, 1.926274f), lampTransform.GetRotation());
    
	auto& cameraTransform = *scene->GetTransform()[4];
	ASSERT_EQ(typeid(Transform), typeid(cameraTransform));
	ASSERT_EQ("Camera_TRANSFORM", cameraTransform.GetDef());
	ASSERT_EQ(Float3(1.0f, -1.0f, 2.0f), cameraTransform.GetTranslation());
	ASSERT_EQ(Float3(1.0f, 1.0f, 1.0f), cameraTransform.GetScale());
	ASSERT_EQ(Float4(0.678598f, 0.281084f, 0.678599f, 1.096056f), cameraTransform.GetRotation());
}