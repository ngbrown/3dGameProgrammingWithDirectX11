//***************************************************************************************
// Converted from color.fx by Frank Luna (C) 2011 All Rights Reserved.
// See https://alexjadczak.wordpress.com/2014/05/26/directx-and-the-windows-8-0-sdk-part-2/
//
// Colors geometry.
//***************************************************************************************

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float4 Color : COLOR;
};

float4 main(VertexOut pin) : SV_Target
{
	return pin.Color;
}
