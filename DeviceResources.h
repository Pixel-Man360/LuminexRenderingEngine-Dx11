#pragma once

#include <wrl/client.h>
#include <d3d11.h>

namespace Engine::Graphics
{

    class DeviceResources
    {
    public:
        DeviceResources();
        ~DeviceResources();

        bool Initialize(HWND hwnd, int width, int height);
        void Present();
        void Resize(int width, int height);

        ID3D11Device* GetDevice() const { return m_device.Get(); }
        ID3D11DeviceContext* GetDeviceContext() const { return m_context.Get(); }
        ID3D11RenderTargetView* GetRenderTargetView() const { return m_rtv.Get(); }
        ID3D11DepthStencilView* GetDepthStencilView() const { return m_dsv.Get(); }
		float GetAspectRatio() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }
       

    private:
        Microsoft::WRL::ComPtr<ID3D11Device> m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
        Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_backBuffer;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthBuffer;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;

        HWND m_hwnd = nullptr;
        int m_width = 0;
        int m_height = 0;

        bool CreateDeviceResources();
        bool CreateWindowSizeDependentResources(int width, int height);
    };

} // namespace Engine::Graphics
