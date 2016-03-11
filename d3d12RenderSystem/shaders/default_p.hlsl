
struct PsInput {
    float4 position : SV_POSITION;
    float4 worldPosition : PS_WORLD_POSITION;
    float4 normal : PS_NORMAL;
    float2 texCoord : PS_TEXCOORD;
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
//uint MaxAmbientLightCount = 3;
//uint MaxDirectionalLightCount = 6;
//uint MaxPointLightCount = 64;
//uint MaxSpotLightCount = 64;
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

cbuffer Material : register(b3) {
    float3 diffuse;
    bool hasDiffuseMap;
    float3 emissive;
    bool hasEmissiveMap;
    float3 specular;
    bool hasSpecularMap;
    float shininess;
    bool hasNormalMap;
    float transparency;
    float _pad1;
    float4x4 _pad2[3];
};

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D specularMap : register(t2);
Texture2D emissiveMap : register(t3);
SamplerState staticSampler : register(s0);


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
    float ret = 0;
    if (distance <= attenuation.a) {
        ret = 1.0f / (attenuation.x + attenuation.y * distance + attenuation.z * (distance * distance));
    }
    return ret;
}

float4 main(PsInput input) : SV_TARGET
{
    //return input.texCoord;
    //return diffuse.Sample(staticSampler, input.texCoord);
        
    float3 diffuseColor = hasDiffuseMap ? diffuseMap.Sample(staticSampler, input.texCoord).rgb : diffuse;
    float3 specularColor = hasSpecularMap ? specularMap.Sample(staticSampler, input.texCoord).rgb : specular;
    float3 emissiveColor = hasEmissiveMap ? emissiveMap.Sample(staticSampler, input.texCoord).rgb : emissive;

    float occlusion = 0;
    float3 normal = input.normal.xyz;
    float3 worldPosition = input.worldPosition.xyz;
    float3 viewDirection = normalize(viewPosition.xyz - worldPosition);

    float3 ambientResult = ambientLights[0].color.rgb * diffuseColor * (1 - occlusion);
    float3 diffuseResult = float3(0, 0, 0);
    float3 specularResult = float3(0, 0, 0);
    
    for (uint i = 0; i < directionalLightCount; i++) {
        DirectionalLight directionLight = directionalLights[i];
        diffuseResult += directionLight.color.rgb * diffuseColor * DiffuseCoefficient(normal, directionLight.direction.xyz);
        specularResult += directionLight.color.rgb * specularColor * SpecularCoefficient(normal, directionLight.direction.xyz, viewDirection, shininess);
    }
    for (uint i2 = 0; i2 < pointLightCount; i2++) {
        PointLight pointLight = pointLights[i2];
        float attenuation = Attenuation(pointLight.attenuation, length(pointLight.position.xyz - worldPosition));
        float3 lightDirection = normalize(worldPosition - pointLight.position.xyz);
        diffuseResult += pointLight.color.rgb * diffuseColor * attenuation * DiffuseCoefficient(normal, lightDirection);
        specularResult += pointLight.color.rgb * specularColor * attenuation * SpecularCoefficient(normal, lightDirection, viewDirection, shininess);
    }
    for (uint i3 = 0; i3 < spotLightCount; i3++) {
        SpotLight spotLight = spotLights[i3];
        float attenuation = Attenuation(spotLight.attenuation, length(spotLight.position.xyz - worldPosition));
        float3 lightDirection = normalize(worldPosition - spotLight.position.xyz);
        float angle = acos(dot(lightDirection, spotLight.direction.xyz));
        float angleFalloff = 0;
        if (angle < spotLight.coneShape.y) {
            angleFalloff = angle > spotLight.coneShape.x ? (spotLight.coneShape.y - angle) / (spotLight.coneShape.y - spotLight.coneShape.x) : 1.0;
        }
        diffuseResult += spotLight.color.rgb * diffuseColor * attenuation * angleFalloff * DiffuseCoefficient(normal, lightDirection);
        specularResult += spotLight.color.rgb * specularColor * attenuation * angleFalloff * SpecularCoefficient(normal, lightDirection, viewDirection, shininess);
    }
    float4 ret = { ambientResult + diffuseResult + specularResult + emissiveColor, 1 - transparency };
    return ret;
    
    
}