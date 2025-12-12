#include "Renderer.h"
#include "Shader.h"
#include "Mesh.h"
#include "ConstantBuffer.h"
#include <DirectXMath.h>

using namespace Engine::Graphics;

Renderer::Renderer() = default;
Renderer::~Renderer()
{
    Release();
}

bool Renderer::Initialize(DeviceResources* deviceResources)
{
    if (!deviceResources) return false;
    m_deviceResources = deviceResources;
    return CreateResources();
}

bool Renderer::CreateResources()
{
    ID3D11Device* device = m_deviceResources->GetDevice();
    ID3D11DeviceContext* context = m_deviceResources->GetDeviceContext();
    if (!device || !context) return false;

    m_shader = new Shader();
    m_mesh = new Mesh();
    m_cb = new ConstantBuffer();

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(SimpleVertex, Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(SimpleVertex, Color), D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    if (!m_shader->LoadFromFiles(device, L"SimpleVS.hlsl", L"SimplePS.hlsl", layoutDesc, ARRAYSIZE(layoutDesc)))
    {
        MessageBox(nullptr, L"Failed to load shaders", L"Error", MB_OK);
        return false;
    }

    if (!m_mesh->CreateTriangle(device))
    {
        return false;
    }

    if (!m_cb->Create(device))
    {
        return false;
    }

    return true;
}

void Renderer::Render()
{
    if (!m_deviceResources) return;

    ID3D11DeviceContext* context = m_deviceResources->GetDeviceContext();
    auto rtv = m_deviceResources->GetRenderTargetView();
    auto dsv = m_deviceResources->GetDepthStencilView();
    if (!context || !rtv) return;

    // Clear RTV and DSV
    float color[4] = { m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w };
    context->ClearRenderTargetView(rtv, color);
    if (dsv) context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Update constant buffer with an identity WVProj (for now)
    CBPerObject cbData = {};
    DirectX::XMFLOAT4X4 id;
    DirectX::XMStoreFloat4x4(&id, DirectX::XMMatrixIdentity());
    cbData.WorldViewProj = id;
    m_cb->Update(context, cbData);

    // Bind shader and input layout
    m_shader->Bind(context);

    // Bind constant buffer to vertex shader slot b0
    ID3D11Buffer* cb = m_cb->GetBuffer();
    context->VSSetConstantBuffers(0, 1, &cb);

    // Draw mesh
    m_mesh->Draw(context);

    // Present
    m_deviceResources->Present();
}

void Renderer::SetClearColor(float r, float g, float b, float a)
{
    m_clearColor = { r, g, b, a };
}

void Renderer::Release()
{
    DestroyResources();

    if (m_shader) { delete m_shader; m_shader = nullptr; }
    if (m_mesh) { delete m_mesh; m_mesh = nullptr; }
    if (m_cb) { delete m_cb; m_cb = nullptr; }
}

void Renderer::DestroyResources()
{
    // individual Release() calls would be used if objects manage COM resources externally;
    // our wrappers handle COM lifetime (ComPtr), and delete will call destructors.
}
