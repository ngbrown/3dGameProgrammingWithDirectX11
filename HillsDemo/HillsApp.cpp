//***************************************************************************************
// HillsApp.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates drawing hills using a grid and 2D function to set the height of each vertex.
// Demonstrates dynamic vertex buffers by performing an animated wave simulation where
// the vertex buffers are updated every frame with the new snapshot of the wave simulation.
// Demonstrates 3D lighting with directional, point, and spot lights.
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

#include "Waves.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

// Not to be confused with GeometryGenerator.Vertex
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
};


struct cbLightingPerObject
{
	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mWorldInvTransform;
	XMFLOAT4X4 mWorldViewProj;
	Material mMaterial;
};

struct cbLightingPerFrame
{
	DirectionalLight mDirLight;
	PointLight mPointLight;
	SpotLight mSpotLight;
	XMFLOAT3 mEyePosW;
	float pad;
	Material mMaterial;
};


class HillsApp : public D3DApp
{
public:
	HillsApp(HINSTANCE hInstance);
	~HillsApp();

	bool Init();				// override
	void OnResize();			// override
	void UpdateScene(float dt);	// implement pure virtual
	void ApplyConstantBufferPerObject(const XMMATRIX& worldViewProj, const XMMATRIX& world, const XMMATRIX& worldInvTransform);
	void ApplyConstantBufferPerFrame(const Material& material);
	void DrawScene(); 			// implement pure virtual

	void OnMouseDown(WPARAM btnState, int x, int y);	// override
	void OnMouseUp(WPARAM btnState, int x, int y);		// override
	void OnMouseMove(WPARAM btnState, int x, int y);	// override

private:
	float GetHillHeight(float x, float z) const;
	XMFLOAT3 GetHillNormal(float x, float z) const;
	void BuildGeometryBuffers();
	void BuildLandGeometryBuffers();
	void BuildWavesGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();
	void BuildRasterState();

private:
	ConstantBuffer<cbLightingPerObject> mConstantBufferPerObject;
	ConstantBuffer<cbLightingPerFrame> mConstantBufferPerFrame;
	ID3D11Buffer* mLandVB;
	ID3D11Buffer* mLandIB;
	ID3D11Buffer* mWavesVB;
	ID3D11Buffer* mWavesIB;

	ID3D11RasterizerState* mRasterState;
	ID3D11RasterizerState* mWireframeRS;

	Waves mWaves;
	DirectionalLight mDirLight;
	PointLight mPointLight;
	SpotLight mSpotLight;
	Material mLandMat;
	Material mWavesMat;

	ID3DBlob* mPSBlob;
	ID3DBlob* mVSBlob;
	ID3D11PixelShader* mPixelShader;
	ID3D11VertexShader* mVertexShader;


	ID3D11InputLayout* mInputLayout;

	// Define transformations from local spaces to world space.
	XMFLOAT4X4 mLandWorld;
	XMFLOAT4X4 mWavesWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	UINT mLandIndexCount;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
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
	mLandVB(nullptr),
	mLandIB(nullptr),
	mWavesVB(nullptr),
	mWavesIB(nullptr),
	mRasterState(nullptr),
	mWireframeRS(nullptr),
	mInputLayout(nullptr),
	mLandIndexCount(0),
	mEyePosW(0.0f, 0.0f, 0.0f),
	mTheta(1.5f*MathHelper::Pi),
	mPhi(0.1f*MathHelper::Pi),
	mRadius(80.0f)
{
	mMainWndCaption = L"Hills Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mLandWorld, I);
	XMStoreFloat4x4(&mWavesWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX wavesOffset = XMMatrixTranslation(0.0f, -3.0f, 0.0f);
	XMStoreFloat4x4(&mWavesWorld, wavesOffset);

	// Directional light.
	mDirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	// Point light--position is changed every frame to animate in UpdateScene function.
	mPointLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mPointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mPointLight.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mPointLight.Att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	mPointLight.Range = 25.0f;

	// Spot light--position and direction changed every frame to animate in UpdateScene function
	mSpotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mSpotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	mSpotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSpotLight.Att = XMFLOAT3(1.0f, 0.0f, 0.0f);
	mSpotLight.Spot = 96.0f;
	mSpotLight.Range = 10000.0f;

	mLandMat.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	mLandMat.Diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	mLandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mWavesMat.Ambient = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	mWavesMat.Diffuse = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	mWavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);
}

HillsApp::~HillsApp()
{
	ReleaseCOM(mLandVB);
	ReleaseCOM(mLandIB);
	ReleaseCOM(mWavesVB);
	ReleaseCOM(mWavesIB);
	ReleaseCOM(mInputLayout);
	ReleaseCOM(mWireframeRS);
}

bool HillsApp::Init()
{
	if (!D3DApp::Init())
		return false;

	mWaves.Init(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);

	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();
	BuildRasterState();
	mConstantBufferPerObject.Initialize(md3dDevice);
	mConstantBufferPerFrame.Initialize(md3dDevice);

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

	mEyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	//
	// Every quarter second, generate a random wave.
	//
	static float t_base = 0.0f;
	if ((mTimer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		DWORD i = 5 + rand() % (mWaves.RowCount() - 10);
		DWORD j = 5 + rand() % (mWaves.ColumnCount() - 10);

		float r = MathHelper::RandF(1.0f, 2.0f);

		mWaves.Disturb(i, j, r);
	}

	mWaves.Update(dt);

	//
	// Update the wave vertex buffer with the new solution.
	//

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(md3dImmediateContext->Map(mWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex* v = reinterpret_cast<Vertex*>(mappedData.pData);
	for (UINT i = 0; i < mWaves.VertexCount(); i++)
	{
		v[i].Pos = mWaves[i];
		v[i].Normal = mWaves.Normal(i);
	}

	md3dImmediateContext->Unmap(mWavesVB, 0);

	//
	// Animate the lights
	//

	// Circle light over the land surface.
	mPointLight.Position.x = 70.0f*cosf(0.2f*mTimer.TotalTime());
	mPointLight.Position.z = 70.0f*sinf(0.2f*mTimer.TotalTime());
	mPointLight.Position.y = MathHelper::Max(GetHillHeight(mPointLight.Position.x, mPointLight.Position.z), -3.0f) + 10.0f;

	// The spotlight takes on the camera position and is aimed in the
	// same direction the camera is looking.  In this way, it looks
	// like we are holding a flashlight.
	mSpotLight.Position = mEyePosW;
	XMStoreFloat3(&mSpotLight.Direction, XMVector3Normalize(target - pos));
}

void HillsApp::ApplyConstantBufferPerObject(const XMMATRIX& worldViewProj, const XMMATRIX& world, const XMMATRIX& worldInvTransform)
{
	// Use a constant buffer. Effect framework deprecated
	cbLightingPerObject perObjectCB;
	XMStoreFloat4x4(&perObjectCB.mWorldViewProj, XMMatrixTranspose(worldViewProj));
	XMStoreFloat4x4(&perObjectCB.mWorld, world);
	XMStoreFloat4x4(&perObjectCB.mWorldInvTransform, worldInvTransform);

	mConstantBufferPerObject.Data = perObjectCB;
	mConstantBufferPerObject.ApplyChanges(md3dImmediateContext);
	ID3D11Buffer* buffer = mConstantBufferPerObject.Buffer();
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &buffer);
}

void HillsApp::ApplyConstantBufferPerFrame(const Material& material)
{
	cbLightingPerFrame perFrameCB;
	perFrameCB.mDirLight = mDirLight;
	perFrameCB.mPointLight = mPointLight;
	perFrameCB.mSpotLight = mSpotLight;
	perFrameCB.mEyePosW = mEyePosW;
	perFrameCB.mMaterial = material;

	mConstantBufferPerFrame.Data = perFrameCB;
	mConstantBufferPerFrame.ApplyChanges(md3dImmediateContext);
	ID3D11Buffer* buffer = mConstantBufferPerFrame.Buffer();
	md3dImmediateContext->PSSetConstantBuffers(0, 1, &buffer);
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

	// Set constants
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view*proj;

	//
	// Draw the Land
	//
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mLandVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mLandIB, DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX world = XMLoadFloat4x4(&mLandWorld);
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj = world*view*proj;

	ApplyConstantBufferPerObject(worldViewProj, world, worldInvTranspose);
	ApplyConstantBufferPerFrame(mLandMat);
	md3dImmediateContext->DrawIndexed(mLandIndexCount, 0, 0);

	//
	// Draw the waves.
	//

	md3dImmediateContext->IASetVertexBuffers(0, 1, &mWavesVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mWavesIB, DXGI_FORMAT_R32_UINT, 0);

	world = XMLoadFloat4x4(&mWavesWorld);
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj = world*view*proj;

	ApplyConstantBufferPerObject(worldViewProj, world, worldInvTranspose);
	ApplyConstantBufferPerFrame(mWavesMat);
	md3dImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);

	// Restore default.
//	md3dImmediateContext->RSSetState(nullptr);

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
		// Make each pixel correspond to 0.2 unit in the scene.
		float dx = 0.2f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.2f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 50.0f, 500.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void HillsApp::BuildGeometryBuffers()
{
	BuildLandGeometryBuffers();
	BuildWavesGeometryBuffers();
}

float HillsApp::GetHillHeight(float x, float z) const
{
	return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z));
}

XMFLOAT3 HillsApp::GetHillNormal(float x, float z) const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void HillsApp::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;
	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	mLandIndexCount = grid.Indices.size();

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  
	//

	std::vector<Vertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); i++)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;
		p.y = GetHillHeight(p.x, p.z);

		vertices[i].Pos = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
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
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mLandVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mLandIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mLandIB));
}

void HillsApp::BuildWavesGeometryBuffers()
{
	// Create the vertex buffer.  Note that we allocate space only, as
	// we will be updating the data every time step of the simulation.

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex) * mWaves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(md3dDevice->CreateBuffer(&vbd, nullptr, &mWavesVB));

	// Create the index buffer.  The index buffer is fixed, so we only 
	// need to create and set once.
	
	std::vector<UINT> indices(3 * mWaves.TriangleCount()); // 3 indices per face

	// Iterate over each quad.
	UINT m = mWaves.RowCount();
	UINT n = mWaves.ColumnCount();
	int k = 0;
	for (UINT i = 0; i < m - 1; i++)
	{
		for (DWORD j = 0; j < n - 1; j++)
		{
			indices[k] = i*n + j;
			indices[k + 1] = i*n + j + 1;
			indices[k + 2] = (i + 1)*n + j;

			indices[k + 3] = (i + 1)*n + j;
			indices[k + 4] = i*n + j + 1;
			indices[k + 5] = (i + 1)*n + j + 1;

			k += 6; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mWavesIB));
}

void HillsApp::BuildFX()
{
	// Load cso files and create shaders
	HR(ShaderHelper::LoadCompiledShader(L"LightingPixelShader.cso", &mPSBlob));
	HR(md3dDevice->CreatePixelShader(mPSBlob->GetBufferPointer(), mPSBlob->GetBufferSize(), nullptr, &mPixelShader));

	HR(ShaderHelper::LoadCompiledShader(L"LightingVertexShader.cso", &mVSBlob));
	HR(md3dDevice->CreateVertexShader(mVSBlob->GetBufferPointer(), mVSBlob->GetBufferSize(), nullptr, &mVertexShader));
}

void HillsApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	//wireframeDesc.CullMode = D3D11_CULL_NONE;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;
	HR(md3dDevice->CreateRasterizerState(&wireframeDesc, &mWireframeRS));
}
