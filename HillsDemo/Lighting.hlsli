//=============================================================================
// Lighting.hlsli by Frank Luna (C) 2011 All Rights Reserved.
//
// transforms and lights geometry.
//=============================================================================

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float3 PosW    : POSITION;
	float3 NormalW : NORMAL;
};
