#include "TreeSprite.hlsli"

struct VertexIn
{
	float3 PosW  : POSITION;
	float2 SizeW : SIZE;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	// Just pass data over to geometry shader.
	vout.CenterW = vin.PosW;
	vout.SizeW = vin.SizeW;

	return vout;
}