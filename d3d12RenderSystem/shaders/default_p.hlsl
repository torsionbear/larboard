
struct PSInput {
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
    float4 diffuseEmissive1;
    float4 specularShininess;
    float2x4 _pad1;
    float4x4 _pad2[3];
};

Texture2D diffuse : register(t0);
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

float4 main(PSInput input) : SV_TARGET
{
    //return input.texCoord;
    //return diffuse.Sample(staticSampler, input.texCoord);

    
    float4 diffuseEmissive = diffuse.Sample(staticSampler, input.texCoord);
    diffuseEmissive.a = 0;
    float occlusion = 0;
    float3 normal = input.normal.xyz;
    float3 worldPosition = input.worldPosition.xyz;
    float3 viewDirection = normalize(viewPosition.xyz - worldPosition);

    float3 ambient = ambientLights[0].color.rgb * diffuseEmissive.rgb * (1 - occlusion) + diffuseEmissive.rgb * diffuseEmissive.a;
    float3 diffuse = { 0.0, 0.0, 0.0 };
    float3 specular = { 0.0, 0.0, 0.0 };
    
    
    for (uint i = 0; i < directionalLightCount; i++) {
        DirectionalLight directionLight = directionalLights[i];
        diffuse += directionLight.color.rgb * diffuseEmissive.rgb * DiffuseCoefficient(normal, directionLight.direction.xyz);
        specular += directionLight.color.rgb * specularShininess.rgb * SpecularCoefficient(normal, directionLight.direction.xyz, viewDirection, specularShininess.a);
    }
    for (uint i2 = 0; i2 < pointLightCount; i2++) {
        PointLight pointLight = pointLights[i2];
        float attenuation = Attenuation(pointLight.attenuation, length(pointLight.position.xyz - worldPosition));
        float3 lightDirection = normalize(worldPosition - pointLight.position.xyz);
        diffuse += pointLight.color.rgb * diffuseEmissive.rgb * attenuation * DiffuseCoefficient(normal, lightDirection);
        specular += pointLight.color.rgb * specularShininess.rgb * attenuation * SpecularCoefficient(normal, lightDirection, viewDirection, specularShininess.a);
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
        diffuse += spotLight.color.rgb * diffuseEmissive.rgb * attenuation * angleFalloff * DiffuseCoefficient(normal, lightDirection);
        specular += spotLight.color.rgb * specularShininess.rgb * attenuation * angleFalloff * SpecularCoefficient(normal, lightDirection, viewDirection, specularShininess.a);
    }
    float4 ret = { ambient + diffuse + specular, 1 };
    return ret;
    
    
}