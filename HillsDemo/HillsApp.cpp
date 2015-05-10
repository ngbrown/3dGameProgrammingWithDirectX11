//***************************************************************************************
// HillsApp.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates drawing hills using a grid and 2D function to set the height of each vertex.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//
//***************************************************************************************


#include <d3dApp.h>
#include <MathHelper.h>
#include <ConstantBuffer.h>
#include <ShaderHelper.h>

#include <GeometryGenerator.h>

#include "cbPerObject.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

// Not to be confused with GeometryGenerator.Vertex
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class HillsApp : public D3DApp
{
public:
	HillsApp(HINSTANCE hInstance);
	~HillsApp();

	bool Init();				// override
	void OnResize();			// override
	void UpdateScene(float dt);	// implement pure virtual
	void DrawScene(); 			// implement pure virtual

	void OnMouseDown(WPARAM btnState, int x, int y);	// override
	void OnMouseUp(WPARAM btnState, int x, int y);		// override
	void OnMouseMove(WPARAM btnState, int x, int y);	// override

private:
	float GetHeight(float x, float z) const;
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();
	void BuildRasterState();

private:
	ConstantBuffer<cbPerObject> mObjectConstantBuffer;
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;
	ID3DBlob* mPSBlob;
	ID3DBlob* mVSBlob;
	ID3D11PixelShader* mPixelShader;
	ID3D11VertexShader* mVertexShader;

	ID3D11RasterizerState* mRasterState;

	ID3D11InputLayout* mInputLayout;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
	UINT mGridIndexCount;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	HillsApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}


HillsApp::HillsApp(HINSTANCE hInstance)
	: D3DApp(hInstance),
	mVB(0),
	mIB(0),
	mInputLayout(0),
	mTheta(1.5f*MathHelper::Pi),
	mPhi(0.25f*MathHelper::Pi),
	mRadius(5.0f)
{
	mMainWndCaption = L"Hills Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

HillsApp::~HillsApp()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mInputLayout);
}

bool HillsApp::Init()
{
	if (!D3DApp::Init())
		return false;

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();
	BuildRasterState();
	mObjectConstantBuffer.Initialize(md3dDevice);

	return true;
}

void HillsApp::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void HillsApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void HillsApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set vertex and pixel shaders
	md3dImmediateContext->PSSetShader(mPixelShader, nullptr, 0);
	md3dImmediateContext->VSSetShader(mVertexShader, nullptr, 0);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	// transpose WVP matrix unless vertix shader is expecting row_major format
	XMMATRIX worldViewProj = XMMatrixTranspose(world*view*proj);

	// Use a constant buffer. Effect framework deprecated
	cbPerObject mPerObjectCB;
	XMStoreFloat4x4(&mPerObjectCB.mWorldViewProj, worldViewProj);
	mObjectConstantBuffer.Data = mPerObjectCB;
	mObjectConstantBuffer.ApplyChanges(md3dImmediateContext);

	auto buffer = mObjectConstantBuffer.Buffer();
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &buffer);

	// Set raster state
	md3dImmediateContext->RSSetState(mRasterState);

	// 36 indices for the box.
	md3dImmediateContext->DrawIndexed(mGridIndexCount, 0, 0);

	HR(mSwapChain->Present(0, 0));
}

void HillsApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void HillsApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void HillsApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

float HillsApp::GetHeight(float x, float z) const
{
	return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z));
}

void HillsApp::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData grid;
	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	mGridIndexCount = grid.Indices.size();

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  In addition, color the vertices based on their height so we have
	// sandy looking beaches, grassy low hills, and snow mountain peaks.
	//

	std::vector<Vertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); i++)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;
		p.y = GetHeight(p.x, p.z);

		vertices[i].Pos = p;

		// Color the vertex based on its height.
		if (p.y < -10.0f)
		{
			// Sandy beach color.
			vertices[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
		}
		else if (p.y < 5.0f)
		{
			// Light yellow-green.
			vertices[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		}
		else if (p.y < 12.0f)
		{
			// Dark yellow-green
			vertices[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
		}
		else if (p.y < 20.0f)
		{
			// Dark brown.
			vertices[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		}
		else
		{
			// White snow
			vertices[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mGridIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}

void HillsApp::BuildFX()
{
	// Load cso files and create shaders
	HR(ShaderHelper::LoadCompiledShader("SimplePixelShader.cso", &mPSBlob));
	HR(md3dDevice->CreatePixelShader(mPSBlob->GetBufferPointer(), mPSBlob->GetBufferSize(), nullptr, &mPixelShader));

	HR(ShaderHelper::LoadCompiledShader("SimpleVertexShader.cso", &mVSBlob));
	HR(md3dDevice->CreateVertexShader(mVSBlob->GetBufferPointer(), mVSBlob->GetBufferSize(), nullptr, &mVertexShader));
}

void HillsApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Create the input layout
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, mVSBlob->GetBufferPointer(),
		mVSBlob->GetBufferSize(), &mInputLayout));
}

void HillsApp::BuildRasterState()
{
	D3D11_RASTERIZER_DESC rs;
	memset(&rs, 0, sizeof(rs));
	rs.FillMode = D3D11_FILL_SOLID;
	rs.CullMode = D3D11_CULL_BACK;
	rs.AntialiasedLineEnable = rs.DepthClipEnable = true;
	mRasterState = nullptr;
	HR(md3dDevice->CreateRasterizerState(&rs, &mRasterState));
}
