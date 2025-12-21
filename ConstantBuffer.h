#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace Engine::Graphics
{
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
