#include "Renderer.h"
#include "Shader.h"
#include "Mesh.h"
#include "ConstantBuffer.h"

#include <DirectXMath.h>
#include <WICTextureLoader.h>

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

    m_cbPerObject = new ConstantBuffer();
    m_cbLight = new ConstantBuffer();

    // -----------------------------
    // Input Layout
    // -----------------------------
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
    // Render Objects
    // -----------------------------
    {
        RenderObject* cube1 = new RenderObject(m_mesh);
        cube1->GetTransform().SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
        m_renderObjects.push_back(cube1);

        RenderObject* cube2 = new RenderObject(m_mesh);
        cube2->GetTransform().SetPosition(XMFLOAT3(3.0f, 0.0f, 0.0f));
        m_renderObjects.push_back(cube2);
    }

    // -----------------------------
    // Constant Buffers
    // -----------------------------
    if (!m_cbPerObject->Create(device, sizeof(CBPerObject)))
        return false;

    if (!m_cbLight->Create(device, sizeof(CBLight)))
        return false;

    // -----------------------------
    // Texture
    // -----------------------------
    if (FAILED(CreateWICTextureFromFile(
        device,
        context,
        L"Assets/textures/Brick.png",
        nullptr,
        &m_diffuseTexture)))
    {
        MessageBox(nullptr, L"Failed to load texture", L"Error", MB_OK);
        return false;
    }

    // -----------------------------
    // Sampler
    // -----------------------------
    D3D11_SAMPLER_DESC samp = {};
    samp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samp.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samp.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samp.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samp.MaxLOD = D3D11_FLOAT32_MAX;

    if (FAILED(device->CreateSamplerState(&samp, &m_samplerState)))
        return false;

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
    ID3D11DeviceContext* context = m_deviceResources->GetDeviceContext();
    ID3D11RenderTargetView* rtv = m_deviceResources->GetRenderTargetView();
    ID3D11DepthStencilView* dsv = m_deviceResources->GetDepthStencilView();

    float clearColor[4] =
    {
        m_clearColor.x,
        m_clearColor.y,
        m_clearColor.z,
        m_clearColor.w
    };

    context->ClearRenderTargetView(rtv, clearColor);
    if (dsv)
        context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

    context->RSSetState(m_rasterizerState);
    context->OMSetDepthStencilState(m_depthStencilState, 0);

    // -----------------------------
    // Camera + Matrices
    // -----------------------------
    float dt = 0.016f; // temporary

    m_camera.Update(dt);

    XMMATRIX view = m_camera.GetViewMatrix();
    XMMATRIX proj = XMMatrixPerspectiveFovLH(
        XM_PIDIV4,
        m_deviceResources->GetAspectRatio(),
        0.1f,
        100.0f);

    // -----------------------------
    // Lighting
    // -----------------------------
    CBLight cbLight = {};
    cbLight.LightDirection = XMFLOAT3(-0.5f, -1.0f, 0.5f);
    cbLight.LightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
    m_cbLight->Update(context, &cbLight);

    // -----------------------------
    // Bind pipeline
    // -----------------------------
    m_shader->Bind(context);

    ID3D11Buffer* vsCB = m_cbPerObject->Get();
    context->VSSetConstantBuffers(0, 1, &vsCB);

    ID3D11Buffer* psCB = m_cbLight->Get();
    context->PSSetConstantBuffers(1, 1, &psCB);

    context->PSSetShaderResources(0, 1, &m_diffuseTexture);
    context->PSSetSamplers(0, 1, &m_samplerState);

    // -----------------------------
    // Draw objects
    // -----------------------------
    for (size_t i = 0; i < m_renderObjects.size(); ++i)
    {
        RenderObject* obj = m_renderObjects[i];

        XMFLOAT3 axis = (i == 0)
            ? XMFLOAT3(0, 1, 0)
            : XMFLOAT3(1, 0, 0);

        obj->GetTransform().RotateAxisAngle(axis, dt * (i + 1));


        XMMATRIX world = obj->GetTransform().GetWorldMatrix();

        // Compute inverse transpose (ignore translation)
        XMMATRIX worldInvTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, world));

        CBPerObject cbObj = {};
        XMStoreFloat4x4(&cbObj.World, XMMatrixTranspose(world));
        XMStoreFloat4x4(&cbObj.WorldInvTranspose, XMMatrixTranspose(worldInvTranspose));
        XMStoreFloat4x4(&cbObj.View, XMMatrixTranspose(view));
        XMStoreFloat4x4(&cbObj.Projection, XMMatrixTranspose(proj));

        m_cbPerObject->Update(context, &cbObj);

        obj->GetMesh()->Draw(context);
    }

    m_deviceResources->Present();
}

void Renderer::SetClearColor(float r, float g, float b, float a)
{
    m_clearColor = { r, g, b, a };
}

void Renderer::Release()
{
    if (m_rasterizerState)  m_rasterizerState->Release();
    if (m_depthStencilState) m_depthStencilState->Release();
    if (m_diffuseTexture)   m_diffuseTexture->Release();
    if (m_samplerState)     m_samplerState->Release();

    delete m_shader;
    delete m_mesh;
    delete m_cbPerObject;
    delete m_cbLight;

    for (RenderObject* obj : m_renderObjects)
        delete obj;

    m_renderObjects.clear();
}
