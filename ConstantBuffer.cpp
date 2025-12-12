#include "ConstantBuffer.h"
#include <cstring>

using namespace Engine::Graphics;

bool ConstantBuffer::Create(ID3D11Device* device)
{
    if (!device) return false;

    D3D11_BUFFER_DESC cbd = {};
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.ByteWidth = sizeof(CBPerObject);
    cbd.CPUAccessFlags = 0;
    cbd.Usage = D3D11_USAGE_DEFAULT;

    HRESULT hr = device->CreateBuffer(&cbd, nullptr, m_buffer.GetAddressOf());
    return SUCCEEDED(hr);
}

void ConstantBuffer::Update(ID3D11DeviceContext* context, const CBPerObject& data)
{
    if (!context || !m_buffer) return;
    context->UpdateSubresource(m_buffer.Get(), 0, nullptr, &data, 0, 0);
}

void ConstantBuffer::Release()
{
    m_buffer.Reset();
}
