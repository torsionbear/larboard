
struct PsInput {
    float4 position : SV_POSITION;
    float2 texCoord : PS_TEXCOORD;
    float3 viewDirection : PS_VIEW_DIRECTION;
};

struct PsOutput {
    float4 color : SV_TARGET;
    float depth : SV_DEPTH;
};

cbuffer Camera : register(b0) {
    float4x4 viewTransform;
    float4x4 projectTransform;
    float4x4 viewTransformInverse;
    float4 viewPosition;
    float3x4 _pad;
};

struct AmbientLight {
    float4 color;
};
struct DirectionalLight {
    float4 color;
    float4 direction;
};
struct PointLight {
    float4 color;
    float4 position;
    float4 attenuation;
};
struct SpotLight {
    float4 color;
    float4 position;
    float4 direction;
    float4 attenuation;
    float4 coneShape; // beamWidth, cutOffAngle, unused, unused
};

cbuffer Lights : register(b2) {
    uint ambientLightCount;
    uint directionalLightCount;
    uint pointLightCount;
    uint spotLightCount;
    AmbientLight ambientLights[3];
    DirectionalLight directionalLights[6];
    PointLight pointLights[64];
    SpotLight spotLights[64];
};

cbuffer SsaoData : register(b3) {
    //int2 randomVectorTextureSize;
    int2 occlusionTextureSize;
    int sampleCount;
    float3 samples[95];
};
static int2 randomVectorTextureSize = int2(4, 4);

cbuffer ShadowCastingLight : register(b5) {
    float4x4 lightViewTransform;
    float4x4 lightProjectTransform;
    float4x4 lightViewTransformInverse;
    float4 lightViewPosition;
    float3x4 _padShadowCastingLight;
};

cbuffer TextureIndex : register(b6) {
    int diffuseMapIndex;
    int normalMapIndex;
    int specularMapIndex;
    int emissiveMapIndex;
    int shadowMapIndex;
    int occlusionIndex;
    int DepthIndex;
};

Texture2D textures[10] : register(t1);
SamplerState staticSampler : register(s0);
SamplerState shadowMapSampler : register(s1);

float screenSpaceDepthToViewSpaceDepth(float screenSpaceDepth) {
    float ndcDepth = screenSpaceDepth;// *2 - 1;
    return -projectTransform[2][3] / (projectTransform[2][2] + ndcDepth);
}

float4 CalculateViewSpacePosition(float depth, float3 viewDirection) {
    float viewCoordZ = screenSpaceDepthToViewSpaceDepth(depth);
    return float4(viewDirection * -viewCoordZ, 1);
}

float GetOcclusion(float2 texCoord) {
    // blur gBuffer.occlusion by randomVectorTextureSize
    float2 texelSize = 1.0 / occlusionTextureSize;
    float occlusion = 0;
    for (int i = 0; i < randomVectorTextureSize.x; ++i) {
        for (int j = 0; j < randomVectorTextureSize.y; ++j) {
            float2 offset = (float2(float(i), float(j)) - randomVectorTextureSize * 0.5) * texelSize;
            occlusion += textures[occlusionIndex].Sample(staticSampler, texCoord + offset).r;
        }
    }
    return occlusion / (randomVectorTextureSize.x * randomVectorTextureSize.y);
}

float DiffuseCoefficient(float3 normal, float3 lightDirection) {
    return max(dot(normal, -lightDirection), 0.0);
}

float SpecularCoefficient(float3 normal, float3 lightDirection, float3 viewDirection, float shininess) {
    float cosine = max(dot(viewDirection, reflect(lightDirection, normal)), 0);
    // multiple 128 to x3d shininess. see http://www.web3d.org/documents/specifications/19775-1/V3.3/Part01/components/lighting.html#Lightingmodel
    shininess *= 128;
    return shininess > 0 ? pow(cosine, shininess) : 1; // avoid pow(0, 0)
}

float Attenuation(float4 attenuation, float distance) {
    if (distance > attenuation[3]) {
        return 0;
    }
    return 1.0f / (attenuation[0] + attenuation[1] * distance + attenuation[2] * (distance * distance));
}

PsOutput main(PsInput input) {
    float depth = textures[DepthIndex].Sample(staticSampler, input.texCoord).r;
    float3 worldPosition = mul(viewTransformInverse, CalculateViewSpacePosition(depth, input.viewDirection)).xyz;
    float3 viewDirection = normalize(viewPosition.xyz - worldPosition.xyz);

    float occlusion = GetOcclusion(input.texCoord);
    float4 diffuseEmissive = textures[diffuseMapIndex].Sample(staticSampler, input.texCoord);
    float4 specularShininess = textures[specularMapIndex].Sample(staticSampler, input.texCoord);
    float3 normal = textures[normalMapIndex].Sample(staticSampler, input.texCoord).rgb;

    float3 ambient = ambientLights[0].color.rgb * diffuseEmissive.rgb *(1 - occlusion) + diffuseEmissive.rgb * diffuseEmissive.a;
    float3 diffuse = float3(0, 0, 0);
    float3 specular = float3(0, 0, 0);

    // shadow
    float bias = 0.001;
    float4 lightSpacePosition = mul(lightViewTransform, float4(worldPosition, 1));
    float4 lightSpaceNdcPosition = mul(lightProjectTransform, lightSpacePosition);
    float2 texCoord = float2((lightSpaceNdcPosition.x + 1) / 2, 1 - (lightSpaceNdcPosition.y + 1) / 2);
    float shadowMapDepth = textures[shadowMapIndex].Sample(shadowMapSampler, texCoord).r;
    bool inShadow = false;
    if (lightSpaceNdcPosition.z < 1.0 && lightSpaceNdcPosition.z - bias > shadowMapDepth) {
        inShadow = true;
    }
    if (!inShadow) {
        for (uint i = 0; i < directionalLightCount; i++) {
            DirectionalLight directionalLight = directionalLights[i];
            diffuse += directionalLight.color.rgb * diffuseEmissive.rgb * DiffuseCoefficient(normal, directionalLight.direction.xyz);
            specular += directionalLight.color.rgb * specularShininess.rgb * SpecularCoefficient(normal, directionalLight.direction.xyz, viewDirection, specularShininess.a);
        }
    }
    for (uint i = 0; i < pointLightCount; i++) {
        PointLight pointLight = pointLights[i];
        float attenuation = Attenuation(pointLight.attenuation, length(pointLight.position.xyz - worldPosition));
        float3 lightDirection = normalize(worldPosition - pointLight.position.xyz);
        diffuse += pointLight.color.rgb * diffuseEmissive.rgb * attenuation * DiffuseCoefficient(normal, lightDirection);
        specular += pointLight.color.rgb * specularShininess.rgb * attenuation * SpecularCoefficient(normal, lightDirection, viewDirection, specularShininess.a);
    }
    //for (uint i = 0; i < spotLightCount; i++) {
    //    SpotLight spotLight = spotLights[i];
    //    float attenuation = Attenuation(spotLight.attenuation, length(spotLight.position.xyz - worldPosition));
    //    float3 lightDirection = normalize(worldPosition - spotLight.position.xyz);
    //    float angle = acos(dot(lightDirection, spotLight.direction.xyz));
    //    float angleFalloff = 0;
    //    if (angle < spotLight.coneShape[1]) {
    //        angleFalloff = angle > spotLight.coneShape[0] ? (spotLight.coneShape[1] - angle) / (spotLight.coneShape[1] - spotLight.coneShape[0]) : 1.0;
    //    }
    //    diffuse += spotLight.color.rgb * diffuseEmissive.rgb * attenuation * angleFalloff * DiffuseCoefficient(normal, lightDirection);
    //    specular += spotLight.color.rgb * specularShininess.rgb * attenuation * angleFalloff * SpecularCoefficient(normal, lightDirection, viewDirection, specularShininess.a);
    //}
    PsOutput ret;
    ret.color = float4(ambient + diffuse + specular, 1);
    //ret.color = float4(normal, 0);
    ret.depth = depth;
    return ret;
}