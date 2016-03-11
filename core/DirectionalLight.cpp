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
    auto max = std::numeric_limits<Float32>::max();
    maxVertex(2) = max;
    shadowCullingVolume.SetMaxVertex(maxVertex);

    shadowCullingVolume.Intersect(shadowCasterAabb);
    SetViewVolume(shadowCullingVolume);
}

}