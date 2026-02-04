#include "ConstantBuffer.h"
#include <cstring>

using namespace Engine::Graphics;

bool ConstantBuffer::Create(ID3D11Device* device, size_t size)
{
    if (!device) return false;

    D3D11_BUFFER_DESC desc = {};
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.ByteWidth = (UINT)((size + 15) / 16 * 16);
    desc.CPUAccessFlags = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;

    return SUCCEEDED(device->CreateBuffer(&desc, nullptr, m_buffer.GetAddressOf()));
}

void ConstantBuffer::Update(ID3D11DeviceContext* context, const void* data)
{
    if (!context || !m_buffer) return;
    context->UpdateSubresource(m_buffer.Get(), 0, nullptr, data, 0, 0);
}

void ConstantBuffer::Release()
{
    m_buffer.Reset();
}
