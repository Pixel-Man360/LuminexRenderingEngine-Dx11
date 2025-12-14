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
    m_cb = new ConstantBuffer();


    // -----------------------------
    // Shader & Mesh
    // -----------------------------

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

    if (!m_mesh->CreateCube(device))
    {
        MessageBox(nullptr, L"Failed to Create Triangle", L"Error", MB_OK);
        return false;
    }

    if (!m_cb->Create(device))
    {
        MessageBox(nullptr, L"Failed to Create Device", L"Error", MB_OK);
        return false;
    }

    // -----------------------------
   // Rasterizer State
   // -----------------------------

	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FrontCounterClockwise = FALSE;
	rastDesc.DepthClipEnable = TRUE;

    HRESULT hr = device->CreateRasterizerState(&rastDesc, &m_rasterizerState);
    if (FAILED(hr)) return false;


    // -----------------------------
   // Depth-Stencil State
   // -----------------------------

	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = TRUE;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS;

	hr = device->CreateDepthStencilState(&depthDesc, &m_depthStencilState);
    if (FAILED(hr)) return false;


    return true;
}

void Renderer::Render()
{
    if (!m_deviceResources) return;

    ID3D11DeviceContext* context = m_deviceResources->GetDeviceContext();
    ID3D11RenderTargetView* rtv = m_deviceResources->GetRenderTargetView();
    ID3D11DepthStencilView* dsv = m_deviceResources->GetDepthStencilView();

    if (!context || !rtv) return;

    // Clear RTV and DSV
    float color[4] = 
    {
        m_clearColor.x, 
        m_clearColor.y,
        m_clearColor.z, 
        m_clearColor.w 
    };

    context->ClearRenderTargetView(rtv, color);
    if (dsv) context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Bind pipeline states
	context->RSSetState(m_rasterizerState);
	context->OMSetDepthStencilState(m_depthStencilState, 1);


    // Update Transforms
    m_rotationAngle += 0.01f;

    XMMATRIX world = XMMatrixRotationY(m_rotationAngle);
    XMMATRIX view = XMMatrixLookAtLH
    (
        //Temporary Camera Data
        XMVectorSet(0.0f, 2.0f, -2.0f, 1.0f),
        XMVectorZero(),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	);


	float aspectRatio = m_deviceResources->GetAspectRatio();
	XMMATRIX projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), aspectRatio, 0.1f, 100.0f);

	XMMATRIX wvp = world * view * projection;

    // Store matrix in constant buffer and update
    CBPerObject cbData = {};
    XMStoreFloat4x4(&cbData.WorldViewProj, XMMatrixTranspose(wvp));
    m_cb->Update(context, cbData);

    // Bind shader and input layout
    m_shader->Bind(context);

    // Bind constant buffer to vertex shader slot b0
    ID3D11Buffer* cb = m_cb->GetBuffer();
    context->VSSetConstantBuffers(0, 1, &cb);

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

    if (m_rasterizerState)
    {
        m_rasterizerState->Release();
		m_rasterizerState = nullptr;
    }

    if (m_depthStencilState)
    {
		m_depthStencilState->Release();
        m_depthStencilState = nullptr;
	}

    if (m_shader) { delete m_shader; m_shader = nullptr; }
    if (m_mesh) { delete m_mesh; m_mesh = nullptr; }
    if (m_cb) { delete m_cb; m_cb = nullptr; }
}

void Renderer::DestroyResources()
{
}
