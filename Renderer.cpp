#include "Renderer.h"
#include "Shader.h"
#include "Mesh.h"
#include "ConstantBuffer.h"
#include "CBPerObject.h"
#include "CBLight.h"

#include <DirectXMath.h>
#include <WICTextureLoader.h>

using namespace Engine::Graphics;
using namespace DirectX;


static XMFLOAT3 Normalize(const XMFLOAT3& v)
{
    XMVECTOR vec = XMLoadFloat3(&v);
    vec = XMVector3Normalize(vec);
    XMFLOAT3 result;
    XMStoreFloat3(&result, vec);
    return result;
}


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
    m_shadowShader = new Shader();
    m_mesh = new Mesh();
    m_planeMesh = new Mesh();

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

    if(!m_shadowShader->LoadFromFiles(
        device,
        L"ShadowVS.hlsl",
        L"ShadowPS.hlsl",
        layoutDesc,
        ARRAYSIZE(layoutDesc)))
    {
        MessageBox(nullptr, L"Failed to load shadow shaders", L"Error", MB_OK);
        return false;
    }

    if (!m_mesh->CreateCube(device))
    {
        MessageBox(nullptr, L"Failed to create cube", L"Error", MB_OK);
        return false;
    }

    if (!m_planeMesh->CreatePlane(device))
    {
        MessageBox(nullptr, L"Failed to create plane", L"Error", MB_OK);
        return false;
    }

    // -----------------------------
    // Constant Buffers
    // -----------------------------
    if (!m_cbPerObject->Create(device, sizeof(CBPerObject)))
        return false;

    if (!m_cbLight->Create(device, sizeof(CBLight)))
        return false;

    // -----------------------------
    // Textures (MOVED UP - LOAD BEFORE CREATING OBJECTS)
    // -----------------------------
    if (FAILED(CreateWICTextureFromFile(
        device,
        context,
        L"Assets/textures/Brick.png",
        nullptr,
        &m_brickTexture)))
    {
        MessageBox(nullptr, L"Failed to load brick texture", L"Error", MB_OK);
        return false;
    }

    if (FAILED(CreateWICTextureFromFile(
        device,
        context,
        L"Assets/textures/Ground.png",
        nullptr,
        &m_groundTexture)))
    {
        MessageBox(nullptr, L"Failed to load ground texture", L"Error", MB_OK);
        return false;
    }

    // -----------------------------
    // Render Objects 
    // -----------------------------
    {
        RenderObject* cube1 = new RenderObject(m_mesh);
        cube1->GetTransform().SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
        cube1->SetTexture(m_brickTexture); 
        m_renderObjects.push_back(cube1);

        RenderObject* cube2 = new RenderObject(m_mesh);
        cube2->GetTransform().SetPosition(XMFLOAT3(5.0f, 0.0f, 0.0f));
        cube2->SetTexture(m_brickTexture);  
        m_renderObjects.push_back(cube2);

        RenderObject* ground = new RenderObject(m_planeMesh);
        ground->GetTransform().SetPosition({ 1, -1.6f, 0 });
        ground->SetTexture(m_groundTexture); 
        m_renderObjects.push_back(ground);
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

    CD3D11_RASTERIZER_DESC shadowRast = {};
	shadowRast.FillMode = D3D11_FILL_SOLID;
	shadowRast.CullMode = D3D11_CULL_FRONT;
    shadowRast.DepthBias = 1000;
	shadowRast.DepthBiasClamp = 0.0f;
	shadowRast.SlopeScaledDepthBias = 1.0f;
	shadowRast.DepthClipEnable = TRUE;

    if (FAILED(device->CreateRasterizerState(&shadowRast, &m_shadowRasterizerState)))
		return false;

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

    // -----------------------------
    // Shadow map
    // -----------------------------
	D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = 2048;
	texDesc.Height = 2048;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_shadowMapTexture)))
        return false;

	// DSV for shadow map
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	if (FAILED(device->CreateDepthStencilView(m_shadowMapTexture, &dsvDesc, &m_shadowMapDSV)))
        return false;

	// SRV for shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

    if (FAILED(device->CreateShaderResourceView(m_shadowMapTexture, &srvDesc, &m_shadowMapSRV)))
		return false;

	// Sampler for shadow map
    D3D11_SAMPLER_DESC shadowSamp = {};
    shadowSamp.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    shadowSamp.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    shadowSamp.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    shadowSamp.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    shadowSamp.BorderColor[0] = 1.0f;
    shadowSamp.BorderColor[1] = 1.0f;  
    shadowSamp.BorderColor[2] = 1.0f; 
    shadowSamp.BorderColor[3] = 1.0f;  
    shadowSamp.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    shadowSamp.MinLOD = 0;           
    shadowSamp.MaxLOD = D3D11_FLOAT32_MAX;  

    if(FAILED(device->CreateSamplerState(&shadowSamp, &m_shadowMapSampler)))
		return false;


	// -----------------------------
	// Lights
	// -----------------------------

    Light sun = {};
    sun.Type = LIGHT_DIRECTIONAL;
    sun.Direction = XMFLOAT3(-0.5f, -1.0, 0.5);
    sun.Color = XMFLOAT3(1, 1, 1);
    sun.Intensity = 1.0f;

   /* Light lamp = {};
    lamp.Type = LIGHT_POINT;
    lamp.Position = XMFLOAT3(3, 3, 0);
    lamp.Color = XMFLOAT3(0.25, 0.85f, 0.3f);
    lamp.Range = 5.0f;
    lamp.Intensity = 1.0f;

    Light lamp2 = {};
    lamp2.Type = LIGHT_POINT;
    lamp2.Position = XMFLOAT3(0, 0, -2);
    lamp2.Color = XMFLOAT3(0.85, 0.75f, 0.0f);
    lamp2.Range = 5.0f;
    lamp2.Intensity = 1.0f;

    Light lamp3 = {};
    lamp3.Type = LIGHT_POINT;
    lamp3.Position = XMFLOAT3(0, 0, 2);
    lamp3.Color = XMFLOAT3(0.5, 0.0f, 0.85f);
    lamp3.Range = 5.0f;
    lamp3.Intensity = 1.0f;*/

    m_lights.push_back(sun);
   /*m_lights.push_back(lamp);
    m_lights.push_back(lamp2);
	m_lights.push_back(lamp3);*/

    return true;
}

void Renderer::Render()
{
    ID3D11DeviceContext* context = m_deviceResources->GetDeviceContext();

    ShadowPass();
    ID3D11RenderTargetView* rtv = m_deviceResources->GetRenderTargetView();
    ID3D11DepthStencilView* dsv = m_deviceResources->GetDepthStencilView();
    context->OMSetRenderTargets(1, &rtv, dsv);

    D3D11_VIEWPORT vp{};
    vp.Width = (float)m_deviceResources->GetWidth();
    vp.Height = (float)m_deviceResources->GetHeight();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    context->RSSetViewports(1, &vp);


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
   
    
    CBLight cb = {};
    cb.LightCount = (int)m_lights.size();
    cb.CameraPosition = m_camera.GetPosition();

    for (int i = 0; i < cb.LightCount; i++)
    {
        cb.Lights[i] = m_lights[i];
    }

    m_cbLight->Update(context, &cb);

    // -----------------------------
    // Bind pipeline
    // -----------------------------
    m_shader->Bind(context);

    ID3D11Buffer* vsCB = m_cbPerObject->Get();
    context->VSSetConstantBuffers(0, 1, &vsCB);

    ID3D11Buffer* psCB = m_cbLight->Get();
    context->PSSetConstantBuffers(1, 1, &psCB);

    context->PSSetShaderResources(1, 1, &m_shadowMapSRV);
    context->PSSetSamplers(0, 1, &m_samplerState);
    context->PSSetSamplers(1, 1, &m_shadowMapSampler);


    // -----------------------------
    // Draw objects
    // -----------------------------
    for (size_t i = 0; i < m_renderObjects.size(); ++i)
    {
        RenderObject* obj = m_renderObjects[i];

        if (i < m_renderObjects.size() - 1)
        {
            XMFLOAT3 axis = (i == 0)
                ? XMFLOAT3(0, 1, 0)
                : XMFLOAT3(1, 0, 0);


            obj->GetTransform().RotateAxisAngle(axis, dt * (i + 1));
        }

       

        XMMATRIX world = obj->GetTransform().GetWorldMatrix();

        // Compute inverse transpose (ignore translation)
        XMMATRIX worldInvTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, world));

        CBPerObject cbObj = {};
        XMStoreFloat4x4(&cbObj.World, XMMatrixTranspose(world));
        XMStoreFloat4x4(&cbObj.WorldInvTranspose, XMMatrixTranspose(worldInvTranspose));
        XMStoreFloat4x4(&cbObj.View, XMMatrixTranspose(view));
        XMStoreFloat4x4(&cbObj.Projection, XMMatrixTranspose(proj));
        XMStoreFloat4x4(&cbObj.LightViewProj, XMMatrixTranspose(m_lightView * m_lightProj));


        m_cbPerObject->Update(context, &cbObj);

        ID3D11ShaderResourceView* texture = obj->GetTexture();
        if (texture)
        {
            context->PSSetShaderResources(0, 1, &texture);
        }

        obj->GetMesh()->Draw(context);
    }

    m_deviceResources->Present();
}

void Renderer::ShadowPass()
{

    auto context = m_deviceResources->GetDeviceContext();

    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    context->PSSetShaderResources(1, 1, nullSRV);
	context->RSSetState(m_shadowRasterizerState);


    context->OMSetRenderTargets(0, nullptr, m_shadowMapDSV);
    context->ClearDepthStencilView(m_shadowMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Shadow viewport
    D3D11_VIEWPORT vp{};
    vp.Width = 2048;
    vp.Height = 2048;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    context->RSSetViewports(1, &vp);


    // Light matrices
    XMFLOAT3 dir = m_lights[0].Direction;
    XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&dir));
    XMVECTOR lightPos = -lightDir * 15.0f;

    m_lightView = XMMatrixLookAtLH(
        lightPos,
        XMVectorZero(),
        XMVectorSet(0, 1, 0, 0)
    );

    m_lightProj = XMMatrixOrthographicLH(15.0f, 15.0f, 0.1f, 50.0f);

    m_shadowShader->Bind(context);
    context->PSSetShader(nullptr, nullptr, 0);

    ID3D11Buffer* vsCB = m_cbPerObject->Get();
    context->VSSetConstantBuffers(0, 1, &vsCB);

	XMMATRIX lightVP = m_lightView * m_lightProj;

    for (auto obj : m_renderObjects)
    {
        XMMATRIX world = obj->GetTransform().GetWorldMatrix();

        CBPerObject cb{};
        XMStoreFloat4x4(&cb.World, XMMatrixTranspose(world));
        XMStoreFloat4x4(&cb.LightViewProj, XMMatrixTranspose(lightVP));

        m_cbPerObject->Update(context, &cb);
        obj->GetMesh()->Draw(context);
    }

    vp.Width = 2048;
    vp.Height = 2048;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    context->RSSetViewports(1, &vp);

}



void Renderer::SetClearColor(float r, float g, float b, float a)
{
    m_clearColor = { r, g, b, a };
}

void Renderer::Release()
{
    if (m_rasterizerState)  m_rasterizerState->Release();
    if (m_depthStencilState) m_depthStencilState->Release();
    if (m_brickTexture)   m_brickTexture->Release();     
    if (m_groundTexture)  m_groundTexture->Release();   
    if (m_samplerState)     m_samplerState->Release();
    if (m_shadowMapTexture) m_shadowMapTexture->Release();
    if (m_shadowMapDSV)     m_shadowMapDSV->Release();
    if (m_shadowMapSRV)     m_shadowMapSRV->Release();
    if (m_shadowMapSampler) m_shadowMapSampler->Release();
    if (m_shadowRasterizerState) m_shadowRasterizerState->Release();

    delete m_shader;
    delete m_mesh;
    delete m_planeMesh;
    delete m_cbPerObject;
    delete m_cbLight;
    delete m_shadowShader;

    for (RenderObject* obj : m_renderObjects)
        delete obj;

    m_renderObjects.clear();
}
