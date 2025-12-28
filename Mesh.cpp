#include "Mesh.h"
#include <stdexcept>

using namespace Engine::Graphics;


bool Mesh::CreateCube(ID3D11Device* device)
{
    if (!device)
    {
        return false;
    }

    // ----------------------------
    // 8 UNIQUE VERTICES
    // ----------------------------
    Vertex vertices[] =
    {
        // Front (+Z)
        {{-1,-1,-1},{0,0,-1},{0,1}},
        {{-1, 1,-1},{0,0,-1},{0,0}},
        {{ 1, 1,-1},{0,0,-1},{1,0}},
        {{ 1,-1,-1},{0,0,-1},{1,1}},

        // Back (-Z)
        {{ 1,-1, 1},{0,0,1},{0,1}},
        {{ 1, 1, 1},{0,0,1},{0,0}},
        {{-1, 1, 1},{0,0,1},{1,0}},
        {{-1,-1, 1},{0,0,1},{1,1}},

        // Left (-X)
        {{-1,-1, 1},{-1,0,0},{0,1}},
        {{-1, 1, 1},{-1,0,0},{0,0}},
        {{-1, 1,-1},{-1,0,0},{1,0}},
        {{-1,-1,-1},{-1,0,0},{1,1}},

        // Right (+X)
        {{ 1,-1,-1},{1,0,0},{0,1}},
        {{ 1, 1,-1},{1,0,0},{0,0}},
        {{ 1, 1, 1},{1,0,0},{1,0}},
        {{ 1,-1, 1},{1,0,0},{1,1}},
        
        // Top (+Y)
         {{-1,1,-1},{0,1,0},{0,1}},
         {{-1,1, 1},{0,1,0},{0,0}},
         {{ 1,1, 1},{0,1,0},{1,0}},
         {{ 1,1,-1},{0,1,0},{1,1}},
         
         // Bottom (-Y)
         {{-1,-1, 1},{0,-1,0},{0,1}},
         {{-1,-1,-1},{0,-1,0},{0,0}},
         {{ 1,-1,-1},{0,-1,0},{1,0}},
         {{ 1,-1, 1},{0,-1,0},{1,1}},
    };

    // ----------------------------
    // 36 INDICES (12 TRIANGLES)
    // ----------------------------

    uint32_t indices[] =
    {
        0,1,2, 0,2,3,
        4,5,6, 4,6,7,
        8,9,10, 8,10,11,
        12,13,14, 12,14,15,
        16,17,18, 16,18,19,
        20,21,22, 20,22,23
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

bool Mesh::CreatePlane(ID3D11Device* device)
{
    Vertex vertices[] =
    {
        {{-10, 0, -10}, {0, 1, 0}, {0, 1}},
        {{-10, 0,  10}, {0, 1, 0}, {0, 0}},
        {{ 10, 0,  10}, {0, 1, 0}, {1, 0}},
        {{ 10, 0, -10}, {0, 1, 0}, {1, 1}},
    };

    uint32_t indices[] =
    {
        0, 1, 2,
        0, 2, 3
    };

    m_indexCount = 6;

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;

    device->CreateBuffer(&vbDesc, &vbData, m_vertexBuffer.GetAddressOf());

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;

    device->CreateBuffer(&ibDesc, &ibData, m_indexBuffer.GetAddressOf());

    return true;
}



void Mesh::Draw(ID3D11DeviceContext* context)
{
    if (!context || !m_vertexBuffer) return;

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->DrawIndexed(m_indexCount, 0, 0);
}

void Mesh::Release()
{
    m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
	m_indexCount = 0;
}
