#include "DeviceResources.h"
#include <dxgi.h>

using namespace Engine::Graphics;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")

DeviceResources::DeviceResources() = default;

DeviceResources::~DeviceResources() = default;

bool DeviceResources::Initialize(HWND hwnd, int width, int height)
{
    m_hwnd = hwnd;
    m_width = width;
    m_height = height;

    if (!CreateDeviceResources())
    {
        return false;
    }

    if (!CreateWindowSizeDependentResources(width, height))
    {
        return false;
    }

    return true;
}

bool DeviceResources::CreateDeviceResources()
{
    UINT creationFlags = 0;
#if defined(_DEBUG)
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = m_width;
    sd.BufferDesc.Height = m_height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1
    };

    D3D_FEATURE_LEVEL createdLevel;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &sd,
        &m_swapChain,
        &m_device,
        &createdLevel,
        &m_context
    );

    return SUCCEEDED(hr);
}

bool DeviceResources::CreateWindowSizeDependentResources(int width, int height)
{
    m_rtv.Reset();
    m_dsv.Reset();
    m_depthBuffer.Reset();
    m_backBuffer.Reset();

    HRESULT hr = m_swapChain->GetBuffer(
        0, __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(m_backBuffer.GetAddressOf())
    );
    if (FAILED(hr))
    {
        return false;
    }

    hr = m_device->CreateRenderTargetView(m_backBuffer.Get(), nullptr, m_rtv.GetAddressOf());
    if (FAILED(hr))
    {
        return false;
    }

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    hr = m_device->CreateTexture2D(&depthDesc, nullptr, m_depthBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        return false;
    }

    hr = m_device->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, m_dsv.GetAddressOf());
    if (FAILED(hr))
    {
        return false;
    }

    m_context->OMSetRenderTargets(1, m_rtv.GetAddressOf(), m_dsv.Get());

    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = static_cast<FLOAT>(width);
    vp.Height = static_cast<FLOAT>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &vp);

    return true;
}

void DeviceResources::Resize(int width, int height)
{
    if (!m_swapChain)
    {
        return;
    }

    m_context->OMSetRenderTargets(0, nullptr, nullptr);
    m_rtv.Reset();
    m_dsv.Reset();
    m_depthBuffer.Reset();
    m_backBuffer.Reset();

    HRESULT hr = m_swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    if (FAILED(hr))
    {
        // handle error (silently return for now)
        return;
    }

    CreateWindowSizeDependentResources(width, height);

	m_width = width;
	m_height = height;
}

void DeviceResources::Present()
{
    if (m_swapChain)
    {
        m_swapChain->Present(1, 0);
    }
}
