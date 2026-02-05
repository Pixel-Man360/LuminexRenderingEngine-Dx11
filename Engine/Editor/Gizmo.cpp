#include "Gizmo.h"
#include <d3dcompiler.h>

using namespace Engine::Editor;
using namespace Engine::Scene;
using namespace DirectX;
using Microsoft::WRL::ComPtr;


struct GizmoVertex
{
    XMFLOAT3 Position;
    XMFLOAT4 Color;
};


struct GizmoCB
{
    XMFLOAT4X4 WorldViewProj;
};


static const char* g_GizmoVS = R"(
cbuffer CB : register(b0)
{
    float4x4 WorldViewProj;
};

struct VS_INPUT
{
    float3 Pos : POSITION;
    float4 Color : COLOR;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Pos = mul(float4(input.Pos, 1.0f), WorldViewProj);
    output.Color = input.Color;
    return output;
}
)";

static const char* g_GizmoPS = R"(
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    return input.Color;
}
)";

Gizmo::Gizmo() = default;
Gizmo::~Gizmo() = default;

bool Gizmo::Initialize(ID3D11Device* device)
{
    if (!device) return false;
    

    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> errorBlob;
    
    HRESULT hr = D3DCompile(g_GizmoVS, strlen(g_GizmoVS), "GizmoVS", 
                            nullptr, nullptr, "main", "vs_5_0",
                            D3DCOMPILE_ENABLE_STRICTNESS, 0,
                            vsBlob.GetAddressOf(), errorBlob.GetAddressOf());
    if (FAILED(hr)) return false;
    
    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), 
                                    vsBlob->GetBufferSize(), 
                                    nullptr, 
                                    m_vertexShader.GetAddressOf());
    if (FAILED(hr)) return false;
    

    ComPtr<ID3DBlob> psBlob;
    hr = D3DCompile(g_GizmoPS, strlen(g_GizmoPS), "GizmoPS",
                    nullptr, nullptr, "main", "ps_5_0",
                    D3DCOMPILE_ENABLE_STRICTNESS, 0,
                    psBlob.GetAddressOf(), errorBlob.GetAddressOf());
    if (FAILED(hr)) return false;
    
    hr = device->CreatePixelShader(psBlob->GetBufferPointer(),
                                   psBlob->GetBufferSize(),
                                   nullptr,
                                   m_pixelShader.GetAddressOf());
    if (FAILED(hr)) return false;
    

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    
    hr = device->CreateInputLayout(layout, 2, 
                                   vsBlob->GetBufferPointer(),
                                   vsBlob->GetBufferSize(),
                                   m_inputLayout.GetAddressOf());
    if (FAILED(hr)) return false;
    

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(GizmoCB);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    hr = device->CreateBuffer(&cbDesc, nullptr, m_constantBuffer.GetAddressOf());
    if (FAILED(hr)) return false;
    

    D3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.DepthClipEnable = TRUE;
    rastDesc.AntialiasedLineEnable = TRUE;
    
    hr = device->CreateRasterizerState(&rastDesc, m_rasterizerState.GetAddressOf());
    if (FAILED(hr)) return false;
    

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = FALSE;  
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    
    hr = device->CreateDepthStencilState(&dsDesc, m_depthStencilState.GetAddressOf());
    if (FAILED(hr)) return false;
    
    CreateLineBuffers(device);
    
    return true;
}


void Gizmo::CreateLineBuffers(ID3D11Device* device)
{
    std::vector<GizmoVertex> vertices;
    
    XMFLOAT4 red = { 1.0f, 0.2f, 0.2f, 1.0f };
    XMFLOAT4 green = { 0.2f, 1.0f, 0.2f, 1.0f };
    XMFLOAT4 blue = { 0.2f, 0.5f, 1.0f, 1.0f };
    
    float len = GIZMO_SIZE;
    float arrowSize = 0.2f;
    float thickness = 0.02f;  // Offset for thicker lines
    
    // Helper lambda to add a thick line (multiple offset lines)
    auto addThickLine = [&](XMFLOAT3 start, XMFLOAT3 end, XMFLOAT4 color, XMFLOAT3 offset1, XMFLOAT3 offset2) {
        // Center line
        vertices.push_back({ start, color });
        vertices.push_back({ end, color });
        // Offset lines for thickness
        vertices.push_back({ {start.x + offset1.x, start.y + offset1.y, start.z + offset1.z}, color });
        vertices.push_back({ {end.x + offset1.x, end.y + offset1.y, end.z + offset1.z}, color });
        vertices.push_back({ {start.x - offset1.x, start.y - offset1.y, start.z - offset1.z}, color });
        vertices.push_back({ {end.x - offset1.x, end.y - offset1.y, end.z - offset1.z}, color });
        vertices.push_back({ {start.x + offset2.x, start.y + offset2.y, start.z + offset2.z}, color });
        vertices.push_back({ {end.x + offset2.x, end.y + offset2.y, end.z + offset2.z}, color });
        vertices.push_back({ {start.x - offset2.x, start.y - offset2.y, start.z - offset2.z}, color });
        vertices.push_back({ {end.x - offset2.x, end.y - offset2.y, end.z - offset2.z}, color });
    };

    // X axis (red) - thick line
    addThickLine({0, 0, 0}, {len, 0, 0}, red, {0, thickness, 0}, {0, 0, thickness});
    // X arrow head
    vertices.push_back({ {len, 0, 0}, red });
    vertices.push_back({ {len - arrowSize, arrowSize * 0.5f, 0}, red });
    vertices.push_back({ {len, 0, 0}, red });
    vertices.push_back({ {len - arrowSize, -arrowSize * 0.5f, 0}, red });
    vertices.push_back({ {len, 0, 0}, red });
    vertices.push_back({ {len - arrowSize, 0, arrowSize * 0.5f}, red });
    vertices.push_back({ {len, 0, 0}, red });
    vertices.push_back({ {len - arrowSize, 0, -arrowSize * 0.5f}, red });

    // Y axis (green) - thick line
    addThickLine({0, 0, 0}, {0, len, 0}, green, {thickness, 0, 0}, {0, 0, thickness});
    // Y arrow head
    vertices.push_back({ {0, len, 0}, green });
    vertices.push_back({ {arrowSize * 0.5f, len - arrowSize, 0}, green });
    vertices.push_back({ {0, len, 0}, green });
    vertices.push_back({ {-arrowSize * 0.5f, len - arrowSize, 0}, green });
    vertices.push_back({ {0, len, 0}, green });
    vertices.push_back({ {0, len - arrowSize, arrowSize * 0.5f}, green });
    vertices.push_back({ {0, len, 0}, green });
    vertices.push_back({ {0, len - arrowSize, -arrowSize * 0.5f}, green });

    // Z axis (blue) - thick line
    addThickLine({0, 0, 0}, {0, 0, len}, blue, {thickness, 0, 0}, {0, thickness, 0});
    // Z arrow head
    vertices.push_back({ {0, 0, len}, blue });
    vertices.push_back({ {arrowSize * 0.5f, 0, len - arrowSize}, blue });
    vertices.push_back({ {0, 0, len}, blue });
    vertices.push_back({ {-arrowSize * 0.5f, 0, len - arrowSize}, blue });
    vertices.push_back({ {0, 0, len}, blue });
    vertices.push_back({ {0, arrowSize * 0.5f, len - arrowSize}, blue });
    vertices.push_back({ {0, 0, len}, blue });
    vertices.push_back({ {0, -arrowSize * 0.5f, len - arrowSize}, blue });
    
    m_vertexCount = static_cast<UINT>(vertices.size());
   
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(GizmoVertex));
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices.data();
    
    device->CreateBuffer(&vbDesc, &initData, m_lineVertexBuffer.GetAddressOf());
}




void Gizmo::Render(ID3D11DeviceContext* context,
                   SceneObject* selectedObject,
                   const XMMATRIX& view,
                   const XMMATRIX& projection,
                   const XMFLOAT3& cameraPos)
{
    if (!selectedObject || m_mode == GizmoMode::None) return;
    
    // Save current render states to restore after gizmo rendering
    ID3D11RasterizerState* prevRasterizerState = nullptr;
    ID3D11DepthStencilState* prevDepthStencilState = nullptr;
    UINT prevStencilRef = 0;
    context->RSGetState(&prevRasterizerState);
    context->OMGetDepthStencilState(&prevDepthStencilState, &prevStencilRef);

    XMFLOAT3 objPos = selectedObject->GetTransform().GetPosition();
    

    XMVECTOR camPosV = XMLoadFloat3(&cameraPos);
    XMVECTOR objPosV = XMLoadFloat3(&objPos);
    float distance = XMVectorGetX(XMVector3Length(XMVectorSubtract(camPosV, objPosV)));
    float gizmoScale = distance * 0.15f;
    if (gizmoScale < 0.5f) gizmoScale = 0.5f;
    if (gizmoScale > 2.0f) gizmoScale = 2.0f;

    

    XMMATRIX world = XMMatrixScaling(gizmoScale, gizmoScale, gizmoScale) *
                     XMMatrixTranslation(objPos.x, objPos.y, objPos.z);
    
    XMMATRIX wvp = world * view * projection;

    D3D11_MAPPED_SUBRESOURCE mapped;
    context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    GizmoCB* cb = static_cast<GizmoCB*>(mapped.pData);
    XMStoreFloat4x4(&cb->WorldViewProj, XMMatrixTranspose(wvp));
    context->Unmap(m_constantBuffer.Get(), 0);
    

    context->IASetInputLayout(m_inputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    
    UINT stride = sizeof(GizmoVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_lineVertexBuffer.GetAddressOf(), &stride, &offset);
    
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    
    context->RSSetState(m_rasterizerState.Get());
    context->OMSetDepthStencilState(m_depthStencilState.Get(), 0);
    

    context->Draw(m_vertexCount, 0);
    
    // Restore previous render states
    context->RSSetState(prevRasterizerState);
    context->OMSetDepthStencilState(prevDepthStencilState, prevStencilRef);
    
    // Release the references we got
    if (prevRasterizerState) prevRasterizerState->Release();
    if (prevDepthStencilState) prevDepthStencilState->Release();
}

XMVECTOR Gizmo::ScreenToWorldRay(int mouseX, int mouseY,
                                  const XMMATRIX& view,
                                  const XMMATRIX& projection,
                                  int screenWidth, int screenHeight)
{

    float ndcX = (2.0f * mouseX / screenWidth) - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY / screenHeight);
    

    XMMATRIX invProj = XMMatrixInverse(nullptr, projection);
    XMMATRIX invView = XMMatrixInverse(nullptr, view);
    

    XMVECTOR rayClip = XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);
    XMVECTOR rayView = XMVector4Transform(rayClip, invProj);
    rayView = XMVectorSetW(rayView, 0.0f);
    

    XMVECTOR rayWorld = XMVector4Transform(rayView, invView);
    rayWorld = XMVector3Normalize(rayWorld);
    
    return rayWorld;
}

GizmoAxis Gizmo::TestAxisHit(const XMVECTOR& rayOrigin,
                              const XMVECTOR& rayDir,
                              const XMFLOAT3& gizmoPos,
                              float scale)
{
    XMVECTOR gizmoCenter = XMLoadFloat3(&gizmoPos);
    float axisLen = GIZMO_SIZE * scale;
    float threshold = AXIS_HIT_THRESHOLD * scale;
    

    struct AxisTest 
    {
        GizmoAxis axis;
        XMVECTOR direction;
        float distance;
    };
    
    AxisTest axes[] = 
    {
        { GizmoAxis::X, XMVectorSet(1, 0, 0, 0), FLT_MAX },
        { GizmoAxis::Y, XMVectorSet(0, 1, 0, 0), FLT_MAX },
        { GizmoAxis::Z, XMVectorSet(0, 0, 1, 0), FLT_MAX }
    };
    
    GizmoAxis closestAxis = GizmoAxis::None;
    float closestDist = FLT_MAX;
    
    for (auto& test : axes)
    {

        XMVECTOR axisStart = gizmoCenter;
        XMVECTOR axisEnd = XMVectorAdd(gizmoCenter, XMVectorScale(test.direction, axisLen));
        

        XMVECTOR w0 = XMVectorSubtract(rayOrigin, axisStart);
        XMVECTOR u = rayDir;
        XMVECTOR v = XMVectorSubtract(axisEnd, axisStart);
        
        float a = XMVectorGetX(XMVector3Dot(u, u));
        float b = XMVectorGetX(XMVector3Dot(u, v));
        float c = XMVectorGetX(XMVector3Dot(v, v));
        float d = XMVectorGetX(XMVector3Dot(u, w0));
        float e = XMVectorGetX(XMVector3Dot(v, w0));
        
        float denom = a * c - b * b;
        if (fabsf(denom) < 0.0001f) continue;
        
        float s = (b * e - c * d) / denom;
        float t = (a * e - b * d) / denom;
        

        if (t < 0.1f) t = 0.1f;  
        if (t > 1.0f) t = 1.0f;
        
        XMVECTOR closestOnRay = XMVectorAdd(rayOrigin, XMVectorScale(u, s));
        XMVECTOR closestOnAxis = XMVectorAdd(axisStart, XMVectorScale(v, t));
        
        float dist = XMVectorGetX(XMVector3Length(XMVectorSubtract(closestOnRay, closestOnAxis)));
        
        if (dist < threshold && dist < closestDist && s > 0)
        {
            closestDist = dist;
            closestAxis = test.axis;
        }
    }
    
    return closestAxis;
}

bool Gizmo::OnMouseDown(int mouseX, int mouseY,
                        SceneObject* selectedObject,
                        const XMMATRIX& view,
                        const XMMATRIX& projection,
                        const XMFLOAT3& cameraPos,
                        int screenWidth, int screenHeight)
{
    if (!selectedObject || m_mode == GizmoMode::None) return false;
    
    XMFLOAT3 objPos = selectedObject->GetTransform().GetPosition();
    

    XMVECTOR camPosV = XMLoadFloat3(&cameraPos);
    XMVECTOR objPosV = XMLoadFloat3(&objPos);
    float distance = XMVectorGetX(XMVector3Length(XMVectorSubtract(camPosV, objPosV)));
    float gizmoScale = distance * 0.15f;
    if (gizmoScale < 0.5f) gizmoScale = 0.5f;
    if (gizmoScale > 2.0f) gizmoScale = 2.0f;
    

    XMVECTOR rayDir = ScreenToWorldRay(mouseX, mouseY, view, projection, screenWidth, screenHeight);
    XMVECTOR rayOrigin = camPosV;
    

    GizmoAxis hit = TestAxisHit(rayOrigin, rayDir, objPos, gizmoScale);
    
    if (hit != GizmoAxis::None)
    {
        m_activeAxis = hit;
        m_isDragging = true;
        m_transformModified = false;
        m_dragMode = m_mode;  // Remember which mode we started with
        m_lastMouseX = mouseX;
        m_lastMouseY = mouseY;
        m_objectStartPos = objPos;
        m_objectStartRot = selectedObject->GetTransform().GetRotationEuler();
        m_objectStartScale = selectedObject->GetTransform().GetScale();
        return true;
    }
    
    return false;
}

void Gizmo::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY,
                        SceneObject* selectedObject,
                        const XMMATRIX& view,
                        const XMMATRIX& projection,
                        const XMFLOAT3& cameraPos,
                        int screenWidth, int screenHeight)
{
    if (!m_isDragging || !selectedObject) return;
    
    // Mark that we've modified the transform
    if (deltaX != 0 || deltaY != 0)
        m_transformModified = true;
    
    
    float sensitivity = 0.01f;
    
   
    XMFLOAT3 pos = selectedObject->GetTransform().GetPosition();
    XMFLOAT3 rot = selectedObject->GetTransform().GetRotationEuler();
    XMFLOAT3 scale = selectedObject->GetTransform().GetScale();
    

    XMMATRIX invView = XMMatrixInverse(nullptr, view);
    XMVECTOR viewRight = XMVector3Normalize(invView.r[0]);
    XMVECTOR viewUp = XMVector3Normalize(invView.r[1]);
    
    float dx = static_cast<float>(deltaX);
    float dy = static_cast<float>(deltaY);
    
    if (m_mode == GizmoMode::Translate)
    {

        XMVECTOR camPosV = XMLoadFloat3(&cameraPos);
        XMVECTOR objPosV = XMLoadFloat3(&pos);
        float distance = XMVectorGetX(XMVector3Length(XMVectorSubtract(camPosV, objPosV)));
        float moveSensitivity = distance * 0.002f;
        
        switch (m_activeAxis)
        {
        case GizmoAxis::X:
            pos.x += dx * moveSensitivity;
            break;
        case GizmoAxis::Y:
            pos.y -= dy * moveSensitivity;
            break;
        case GizmoAxis::Z:
            pos.z += dx * moveSensitivity;
            break;
        }
        selectedObject->GetTransform().SetPosition(pos);
    }
    else if (m_mode == GizmoMode::Rotate)
    {
        float rotSensitivity = 0.5f;
        switch (m_activeAxis)
        {
        case GizmoAxis::X:
            rot.x += dy * rotSensitivity;
            break;
        case GizmoAxis::Y:
            rot.y += dx * rotSensitivity;
            break;
        case GizmoAxis::Z:
            rot.z += dx * rotSensitivity;
            break;
        }
        selectedObject->GetTransform().SetRotationEuler(rot);
    }
    else if (m_mode == GizmoMode::Scale)
    {
        float scaleSensitivity = 0.01f;
        float delta = dx * scaleSensitivity;
        switch (m_activeAxis)
        {
        case GizmoAxis::X:
            scale.x += delta;
            if (scale.x < 0.01f) scale.x = 0.01f;
            break;
        case GizmoAxis::Y:
            scale.y -= dy * scaleSensitivity;
            if (scale.y < 0.01f) scale.y = 0.01f;
            break;
        case GizmoAxis::Z:
            scale.z += delta;
            if (scale.z < 0.01f) scale.z = 0.01f;
            break;
        }
        selectedObject->GetTransform().SetScale(scale);
    }
    
    m_lastMouseX = mouseX;
    m_lastMouseY = mouseY;
}

bool Gizmo::OnMouseUp(SceneObject* selectedObject)
{
    bool wasModified = m_isDragging && m_transformModified && selectedObject;
    
    m_isDragging = false;
    m_activeAxis = GizmoAxis::None;
    m_transformModified = false;
    
    return wasModified;
}
