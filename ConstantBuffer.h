#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace Engine::Graphics
{

    // Must be 16-byte aligned for HLSL constant buffer rules
    struct alignas(16) CBPerObject
    {
        XMFLOAT4X4 World;
        XMFLOAT4X4 WorldInvTranspose;
		XMFLOAT4X4 View;
        XMFLOAT4X4 Projection;
    };

    struct alignas(16) CBLight
    {
        XMFLOAT3 LightDirection;
        float Padding1; // Padding to ensure 16-byte alignment

        XMFLOAT3 LightColor;
		float padding2;
	};

    class ConstantBuffer
    {
    public:
        bool Create(ID3D11Device* device, size_t size);
        void Update(ID3D11DeviceContext* context, const void* data);
        void Release();
        ID3D11Buffer* Get() const { return m_buffer.Get(); }

    private:
        ComPtr<ID3D11Buffer> m_buffer;
    };

} // namespace Engine::Graphics
