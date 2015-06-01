#include "TreeSprite.hlsli"
#include "LightHelper.hlsli"

cbuffer cbPerFrame
{
	int gLightCount;
	int pad1;
	int pad2;
	int pad3;

	DirectionalLight gDirLights[3];
	float3 gEyePosW;

	float  gFogStart;
	float  gFogRange;
	float4 gFogColor;
}

cbuffer cbPerObject
{
	Material gMaterial;
	bool gUseTexure;
	bool gAlphaClip;
	bool gFogEnabled;
};

// Nonnumeric values cannot be added to a cbuffer.
Texture2DArray gTreeMapArray;

SamplerState samLinear;

float4 main(GeoOut pin) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
	pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;

	// Default to multiplicative identity.
	float4 texColor = float4(1, 1, 1, 1);
	if (gUseTexure)
	{
		// Sample texture.
		float3 uvw = float3(pin.Tex, pin.PrimID % 4);
		texColor = gTreeMapArray.Sample(samLinear, uvw);

		if (gAlphaClip)
		{
			// Discard pixel if texture alpha < 0.05.  Note that we do this
			// test as soon as possible so that we can potentially exit the shader 
			// early, thereby skipping the rest of the shader code.
			clip(texColor.a - 0.05f);
		}
	}

	//
	// Lighting.
	//

	float4 litColor = texColor;
	if (gLightCount > 0)
	{
		// Start with a sum of zero. 
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

		// Sum the light contribution from each light source.
		[unroll]
		for (int i = 0; i < gLightCount; i++)
		{
			float4 A, D, S;
			ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye, A, D, S);

			ambient += A;
			diffuse += D;
			spec += S;
		}

		// Modulate with late add
		litColor = texColor*(ambient + diffuse) + spec;
	}

	//
	// Fogging
	//

	if (gFogEnabled)
	{
		float fogLerp = saturate((distToEye - gFogStart) / gFogRange);

		// Blend the fog color and the lit color.
		litColor = lerp(litColor, gFogColor, fogLerp);
	}

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

	return litColor;
}