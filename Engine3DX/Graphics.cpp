#include "Graphics.h"
// Link the libraries here instead of in the project settings
#pragma comment(lib, "d3d11.lib")

Graphics::Graphics(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	// Create device and front/back buffers, swap chain and context
	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwapChain,
		&pDevice,
		nullptr,
		&pContext
	);

	// Gain access to texture subresource in swap chain
	ID3D11Resource* pBackBuffer = nullptr;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&pBackBuffer));
	// No descriptor, just default
	pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pTargetView);
	pBackBuffer->Release();
}

Graphics::~Graphics()
{
	if (pTargetView != nullptr) {
		pTargetView->Release();
	}
	if (pContext != nullptr) {
		pContext->Release();
	}
	if (pSwapChain != nullptr) {
		pSwapChain->Release();
	}
	if (pDevice != nullptr) {
		pDevice->Release();
	}
}

void Graphics::EndFrame()
{
	// SyncInterval refers to frame rate
	// Set to 2u if you wanna target 30fps instead of 60
	pSwapChain->Present(1u, 0u);
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept
{
	const float color[]{ red, green, blue, 1.0f };
	pContext->ClearRenderTargetView(pTargetView, color);
}
