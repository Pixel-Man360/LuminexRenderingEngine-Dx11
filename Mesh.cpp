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

bool Mesh::CreateCube(ID3D11Device* device)
{
    if (!device)
    {
        return false;
    }

    // ----------------------------
    // 8 UNIQUE VERTICES
    // ----------------------------
    SimpleVertex vertices[] =
    {
        { { -0.5f, -0.5f, -0.5f }, { 1, 0, 0, 1 } }, // 0
        { { -0.5f,  0.5f, -0.5f }, { 0, 1, 0, 1 } }, // 1
        { {  0.5f,  0.5f, -0.5f }, { 0, 0, 1, 1 } }, // 2
        { {  0.5f, -0.5f, -0.5f }, { 1, 1, 0, 1 } }, // 3
        { { -0.5f, -0.5f,  0.5f }, { 1, 0, 1, 1 } }, // 4
        { { -0.5f,  0.5f,  0.5f }, { 0, 1, 1, 1 } }, // 5
        { {  0.5f,  0.5f,  0.5f }, { 0, 0, 1, 1 } }, // 6
        { {  0.5f, -0.5f,  0.5f }, { 0, 0, 1, 1 } }  // 7
    };

    // ----------------------------
    // 36 INDICES (12 TRIANGLES)
    // ----------------------------
    uint16_t indices[] =
    {
        // Front
        4,5,6, 4,6,7,
        // Back
        0,2,1, 0,3,2,
        // Left
        0,1,5, 0,5,4,
        // Right
        3,6,2, 3,7,6,
        // Top
        1,2,6, 1,6,5,
        // Bottom
        0,7,3, 0,4,7
    };

    m_indexCount = _countof(indices);

    // ----------------------------
    // VERTEX BUFFER
    // ----------------------------
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;

    if (FAILED(device->CreateBuffer(&vbDesc, &vbData, m_vertexBuffer.GetAddressOf())))
    {
        return false;
    }

    // ----------------------------
    // INDEX BUFFER
    // ----------------------------
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;

    if (FAILED(device->CreateBuffer(&ibDesc, &ibData, m_indexBuffer.GetAddressOf())))
    {
        return false;
    }

    return true;
}


void Mesh::Draw(ID3D11DeviceContext* context)
{
    if (!context || !m_vertexBuffer) return;

    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->DrawIndexed(m_indexCount, 0, 0);
}

void Mesh::Release()
{
    m_vertexBuffer.Reset();
    m_vertexCount = 0;
	m_indexBuffer.Reset();
	m_indexCount = 0;
}
