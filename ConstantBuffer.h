#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;

namespace Engine::Graphics
{

    // Must be 16-byte aligned for HLSL constant buffer rules
    struct alignas(16) CBPerObject
    {
        DirectX::XMFLOAT4X4 WorldViewProj;
    };

    class ConstantBuffer
    {
    public:
        ConstantBuffer() = default;
        ~ConstantBuffer() = default;

        bool Create(ID3D11Device* device);
        void Update(ID3D11DeviceContext* context, const CBPerObject& data);
        void Release();
        ID3D11Buffer* GetBuffer() const { return m_buffer.Get(); }

    private:
        ComPtr<ID3D11Buffer> m_buffer;
    };

} // namespace Engine::Graphics
