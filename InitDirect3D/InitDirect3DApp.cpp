#include <d3dApp.h>
#include <Trace.h>

using namespace DirectX;

class InitDirect3DApp : public D3DApp
{
public:
	InitDirect3DApp(HINSTANCE hInstance);
	~InitDirect3DApp();

	bool Init() override;
	void OnResize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;

private:
	void CheckAdaptors();
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	InitDirect3DApp theApp(hInstance);

	if (!theApp.Init())
	{
		return 0;
	}

	return theApp.Run();
}


InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

InitDirect3DApp::~InitDirect3DApp()
{
}

bool InitDirect3DApp::Init()
{
	CheckAdaptors();

	if (!D3DApp::Init())
	{
		return false;
	}

	return true;
}

void InitDirect3DApp::OnResize()
{
	D3DApp::OnResize();
}

void InitDirect3DApp::UpdateScene(float dt)
{
}

void InitDirect3DApp::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	// Clear the back buffer blue. Colors::Blue is defined in d3dUtil.h
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue));

	// Clear the depth buffer to 1.0f and the stencil buffer to 0.
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0);

	// Present the back buffer to the screen.
	HR(mSwapChain->Present(0, 0));
}

void InitDirect3DApp::CheckAdaptors()
{
	const D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	IDXGIFactory1* dxgiFactory = nullptr;
	CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));

	IDXGIAdapter* dxgiAdapter = nullptr;
	UINT i = 0;
	while (dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC dxgiAdapterDesc;
		HR(dxgiAdapter->GetDesc(&dxgiAdapterDesc));

		_trace(L"Display Adaptor %i: %s", i, dxgiAdapterDesc.Description);

		D3D_FEATURE_LEVEL maxFeatureLevel;

		HRESULT hr = D3D11CreateDevice(
			dxgiAdapter,
			D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			0,
			feature_levels,
			sizeof(feature_levels) / sizeof(feature_levels[0]),
			D3D11_SDK_VERSION,
			nullptr,
			&maxFeatureLevel,
			nullptr);

		if (FAILED(hr))
		{
			_trace(L"Does not support D3D11CreateDevice");
		}
		else
		{
			switch (maxFeatureLevel)
			{
			case D3D_FEATURE_LEVEL_9_1:
				_trace(L"Max feature level is: D3D_FEATURE_LEVEL_9_1");
				break;
			case D3D_FEATURE_LEVEL_9_2:
				_trace(L"Max feature level is: D3D_FEATURE_LEVEL_9_2");
				break;
			case D3D_FEATURE_LEVEL_9_3:
				_trace(L"Max feature level is: D3D_FEATURE_LEVEL_9_3");
				break;
			case D3D_FEATURE_LEVEL_10_0:
				_trace(L"Max feature level is: D3D_FEATURE_LEVEL_10_0");
				break;
			case D3D_FEATURE_LEVEL_10_1:
				_trace(L"Max feature level is: D3D_FEATURE_LEVEL_10_1");
				break;
			case D3D_FEATURE_LEVEL_11_0:
				_trace(L"Max feature level is: D3D_FEATURE_LEVEL_11_0");
				break;
			case D3D_FEATURE_LEVEL_11_1:
				_trace(L"Max feature level is: D3D_FEATURE_LEVEL_11_1");
				break;
			default:
				_trace(L"Max feature level is: 0x%X", maxFeatureLevel);
				break;
			}
		}

		UINT j = 0;
		IDXGIOutput* dxgiOutput = nullptr;
		while (dxgiAdapter->EnumOutputs(j, &dxgiOutput) != DXGI_ERROR_NOT_FOUND)
		{
			j++;
		}
		_trace(L"%i Outputs for adaptor", j);

		i++;
	}

	ReleaseCOM(dxgiFactory);
}
