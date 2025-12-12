#include "Mesh.h"
#include <stdexcept>

using namespace Engine::Graphics;

bool Mesh::CreateTriangle(ID3D11Device* device)
{
    if (!device) return false;

    // Define 3 vertices (positions in NDC will be transformed by shaders later; here we use simple positions)
    SimpleVertex verts[] =
    {
        { { 0.0f, 0.5f, 0.0f },  { 1.0f, 0.0f, 0.0f, 1.0f } }, // top (red)
        { { 0.5f,-0.5f, 0.0f },  { 0.0f, 1.0f, 0.0f, 1.0f } }, // right (green)
        { { -0.5f,-0.5f,0.0f },  { 0.0f, 0.0f, 1.0f, 1.0f } }  // left (blue)
    };

	m_vertexCount = _countof(verts);

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * m_vertexCount;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = verts;

    HRESULT hr = device->CreateBuffer(&bd, &initData, m_vertexBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    return true;
}

void Mesh::Draw(ID3D11DeviceContext* context)
{
    if (!context || !m_vertexBuffer) return;

    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->Draw(m_vertexCount, 0);
}

void Mesh::Release()
{
    m_vertexBuffer.Reset();
    m_vertexCount = 0;
}
