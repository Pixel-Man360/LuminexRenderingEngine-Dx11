#include "Renderer.h"
#include "Shader.h"
#include "Mesh.h"
#include "ConstantBuffer.h"
#include <DirectXMath.h>

using namespace Engine::Graphics;
using namespace DirectX;

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

    // NEW: two constant buffers
    m_cbPerObject = new ConstantBuffer();
    m_cbLight = new ConstantBuffer();

    // -----------------------------
    // Input Layout
    // -----------------------------
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
          D3D11_INPUT_PER_VERTEX_DATA, 0 },

        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
          D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    if (!m_shader->LoadFromFiles(
        device,
        L"SimpleVS.hlsl",
        L"SimplePS.hlsl",
        layoutDesc,
        ARRAYSIZE(layoutDesc)))
    {
        MessageBox(nullptr, L"Failed to load shaders", L"Error", MB_OK);
        return false;
    }

    if (!m_mesh->CreateCube(device))
    {
        MessageBox(nullptr, L"Failed to create cube", L"Error", MB_OK);
        return false;
    }

    // -----------------------------
    // Constant Buffers
    // -----------------------------
    if (!m_cbPerObject->Create(device, sizeof(CBPerObject)))
    {
        MessageBox(nullptr, L"Failed to create CBPerObject", L"Error", MB_OK);
        return false;
    }

    if (!m_cbLight->Create(device, sizeof(CBLight)))
    {
        MessageBox(nullptr, L"Failed to create CBLight", L"Error", MB_OK);
        return false;
    }

    // -----------------------------
    // Rasterizer State
    // -----------------------------
    D3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_BACK;
    rastDesc.DepthClipEnable = TRUE;

    if (FAILED(device->CreateRasterizerState(&rastDesc, &m_rasterizerState)))
        return false;

    // -----------------------------
    // Depth Stencil State
    // -----------------------------
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D11_COMPARISON_LESS;

    if (FAILED(device->CreateDepthStencilState(&depthDesc, &m_depthStencilState)))
        return false;

    return true;
}

void Renderer::Render()
{
    if (!m_deviceResources) return;

    ID3D11DeviceContext* context = m_deviceResources->GetDeviceContext();
    ID3D11RenderTargetView* rtv = m_deviceResources->GetRenderTargetView();
    ID3D11DepthStencilView* dsv = m_deviceResources->GetDepthStencilView();

    if (!context || !rtv) return;

    float clearColor[4] =
    {
        m_clearColor.x,
        m_clearColor.y,
        m_clearColor.z,
        m_clearColor.w
    };

    context->ClearRenderTargetView(rtv, clearColor);
    if (dsv)
        context->ClearDepthStencilView(dsv,
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.0f, 0);

    // Bind pipeline states
    context->RSSetState(m_rasterizerState);
    context->OMSetDepthStencilState(m_depthStencilState, 0);

    // -----------------------------
    // Matrices
    // -----------------------------
    m_rotationAngle += 0.01f;

    XMMATRIX world = XMMatrixRotationY(m_rotationAngle);

    XMMATRIX view = XMMatrixLookAtLH
    (
        XMVectorSet(0.0f, 2.0f, -5.0f, 1.0f),
        XMVectorZero(),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    );

    float aspect = m_deviceResources->GetAspectRatio();
    XMMATRIX projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 0.1f, 100.0f);

    // -----------------------------
    // Update CBPerObject (VS)
    // -----------------------------
    CBPerObject cbObj = {};
    XMStoreFloat4x4(&cbObj.World, XMMatrixTranspose(world));
    XMStoreFloat4x4(&cbObj.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&cbObj.Projection, XMMatrixTranspose(projection));

    m_cbPerObject->Update(context, &cbObj);

    // -----------------------------
    // Update CBLight (PS)
    // -----------------------------
    CBLight cbLight = {};
    cbLight.LightDirection = XMFLOAT3(-0.5f, -2.0f, 1.5f);
    cbLight.LightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);

    m_cbLight->Update(context, &cbLight);

    // -----------------------------
    // Bind shaders + buffers
    // -----------------------------
    m_shader->Bind(context);

    ID3D11Buffer* vsCB = m_cbPerObject->Get();
    context->VSSetConstantBuffers(0, 1, &vsCB);

    ID3D11Buffer* psCB = m_cbLight->Get();
    context->PSSetConstantBuffers(1, 1, &psCB);

    // -----------------------------
    // Draw
    // -----------------------------
    m_mesh->Draw(context);

    m_deviceResources->Present();
}

void Renderer::SetClearColor(float r, float g, float b, float a)
{
    m_clearColor = { r, g, b, a };
}

void Renderer::Release()
{
    DestroyResources();

    if (m_rasterizerState) { m_rasterizerState->Release(); m_rasterizerState = nullptr; }
    if (m_depthStencilState) { m_depthStencilState->Release(); m_depthStencilState = nullptr; }

    delete m_shader;       m_shader = nullptr;
    delete m_mesh;         m_mesh = nullptr;
    delete m_cbPerObject;  m_cbPerObject = nullptr;
    delete m_cbLight;      m_cbLight = nullptr;
}

void Renderer::DestroyResources()
{
}
