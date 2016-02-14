//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 viewTransform;
    float4x4 projectTransform;
    float4x4 viewTransformInverse;
    float4 viewPosition;
};

PSInput VSMain(float4 position : POSITION, float4 normal : NORMAL, float4 texCoord : TEXCOORD )
{
	PSInput result;

	float4 newPosition = position;
	newPosition = mul(viewTransform, newPosition);
	newPosition = mul(projectTransform, newPosition);
	
	result.position = newPosition;
	float4 color = {0.0f, 1.0f, 0.0f, 1.0f};
	result.color = color;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return input.color;
}
