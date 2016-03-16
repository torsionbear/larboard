#include "DirectionalLight.h"

namespace core {

auto core::DirectionalLight::ComputeShadowMappingVolume(Camera * camera, Aabb shadowCasterAabb) -> void {
    auto viewFrustumVertex = camera->GetViewFrustumVertex();
    auto const& matrixInverse = GetRigidBodyMatrixInverse();

    // for now, use a simple AABB for shadow culling volume.
    auto shadowCullingVolume = Aabb{};
    for (auto & vertex : viewFrustumVertex) {
        auto p = static_cast<Point4f>(matrixInverse * vertex);
        shadowCullingVolume.Expand(p);
    }
    // directional light's shadowCullingVolume has no upper bound
    auto maxVertex = shadowCullingVolume.GetMaxVertex();
    maxVertex(2) = std::numeric_limits<Float32>::max();
    shadowCullingVolume.SetMaxVertex(maxVertex);

    auto lightSpaceShadowCasterAabb = Aabb{};
    auto shadowCasterAabbVertex = shadowCasterAabb.GetVertex();
    for (auto & vertex : shadowCasterAabbVertex) {
        auto p = static_cast<Point4f>(matrixInverse * vertex);
        lightSpaceShadowCasterAabb.Expand(p);
    }
    shadowCullingVolume.Intersect(lightSpaceShadowCasterAabb);
    SetViewVolume(shadowCullingVolume);
}

}