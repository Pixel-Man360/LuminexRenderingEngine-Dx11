#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Gizmo.h"

#include <d3dcompiler.h>
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <cstring>
#include <vector>

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
    XMFLOAT4 OverrideColor;
    XMFLOAT4 Settings;
};

struct GizmoDrawRange
{
    UINT Start = 0;
    UINT Count = 0;
};

static GizmoDrawRange g_translateXRange;
static GizmoDrawRange g_translateYRange;
static GizmoDrawRange g_translateZRange;
static GizmoDrawRange g_translateCenterRange;

static GizmoDrawRange g_scaleXRange;
static GizmoDrawRange g_scaleYRange;
static GizmoDrawRange g_scaleZRange;
static GizmoDrawRange g_scaleCenterRange;

static GizmoDrawRange g_rotateXRange;
static GizmoDrawRange g_rotateYRange;
static GizmoDrawRange g_rotateZRange;
static GizmoDrawRange g_rotateViewRange;

static constexpr int ROTATION_SEGMENTS = 128;
static constexpr float ROTATION_GIZMO_SCALE = 0.6f;
static constexpr float ROTATION_LINE_WIDTH = 0.018f;
static constexpr float ROTATION_HIT_WIDTH = 0.16f;
static constexpr float CENTER_HIT_RADIUS = 14.0f;

static const XMFLOAT4 GIZMO_GRAY = { 0.55f, 0.55f, 0.55f, 1.0f };

static const char* g_GizmoVS = R"(
cbuffer CB : register(b0)
{
    float4x4 WorldViewProj;
    float4 OverrideColor;
    float4 Settings;
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
    output.Color = Settings.x > 0.5f ? OverrideColor : input.Color;
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

static float GetGizmoScale(const XMFLOAT3& cameraPos, const XMFLOAT3& objectPos)
{
    XMVECTOR camera = XMLoadFloat3(&cameraPos);
    XMVECTOR object = XMLoadFloat3(&objectPos);

    float distance = XMVectorGetX(XMVector3Length(camera - object));
    return std::clamp(distance * 0.15f, 0.5f, 2.0f);
}

static XMVECTOR GetAxisVector(GizmoAxis axis)
{
    if (axis == GizmoAxis::X) return XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    if (axis == GizmoAxis::Y) return XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    if (axis == GizmoAxis::Z) return XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

    return XMVectorZero();
}

static XMVECTOR GetViewForward(const XMMATRIX& view)
{
    XMMATRIX inverseView = XMMatrixInverse(nullptr, view);
    return XMVector3Normalize(inverseView.r[2]);
}

static XMFLOAT2 ProjectToScreen(const XMFLOAT3& worldPosition, const XMMATRIX& view,
                                const XMMATRIX& projection, int screenWidth, int screenHeight)
{
    XMVECTOR position = XMLoadFloat3(&worldPosition);

    XMVECTOR projected = XMVector3Project(position, 0.0f, 0.0f,
                                          static_cast<float>(screenWidth), static_cast<float>(screenHeight),
                                          0.0f, 1.0f, projection, view, XMMatrixIdentity());

    return { XMVectorGetX(projected), XMVectorGetY(projected) };
}

static bool IsCenterHit(int mouseX, int mouseY, const XMFLOAT3& position, const XMMATRIX& view,
                        const XMMATRIX& projection, int screenWidth, int screenHeight)
{
    XMFLOAT2 screenPosition = ProjectToScreen(position, view, projection, screenWidth, screenHeight);

    float dx = static_cast<float>(mouseX) - screenPosition.x;
    float dy = static_cast<float>(mouseY) - screenPosition.y;

    return dx * dx + dy * dy <= CENTER_HIT_RADIUS * CENTER_HIT_RADIUS;
}

static bool RayPlaneIntersection(const XMVECTOR& rayOrigin, const XMVECTOR& rayDirection,
                                 const XMVECTOR& planePoint, const XMVECTOR& planeNormal, XMVECTOR& hitPoint)
{
    float denominator = XMVectorGetX(XMVector3Dot(rayDirection, planeNormal));
    if (fabsf(denominator) < 0.00001f) return false;

    float distance = XMVectorGetX(XMVector3Dot(planePoint - rayOrigin, planeNormal)) / denominator;
    if (distance < 0.0f) return false;

    hitPoint = rayOrigin + rayDirection * distance;
    return true;
}

static bool RaySphereIntersection(const XMVECTOR& rayOrigin, const XMVECTOR& rayDirection,
                                  const XMFLOAT3& centerPosition, float radius)
{
    XMVECTOR center = XMLoadFloat3(&centerPosition);
    XMVECTOR offset = rayOrigin - center;

    float a = XMVectorGetX(XMVector3Dot(rayDirection, rayDirection));
    float b = 2.0f * XMVectorGetX(XMVector3Dot(offset, rayDirection));
    float c = XMVectorGetX(XMVector3Dot(offset, offset)) - radius * radius;

    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f) return false;

    float distance = (-b - sqrtf(discriminant)) / (2.0f * a);

    if (distance < 0.0f)
    {
        distance = (-b + sqrtf(discriminant)) / (2.0f * a);
        if (distance < 0.0f) return false;
    }

    return true;
}

static bool IsSelectedObjectHit(const XMVECTOR& rayOrigin, const XMVECTOR& rayDirection, SceneObject* selectedObject)
{
    if (!selectedObject || !selectedObject->GetMesh()) return false;

    XMFLOAT3 position = selectedObject->GetTransform().GetPosition();
    XMFLOAT3 scale = selectedObject->GetTransform().GetScale();

    float maxScale = std::max(scale.x, std::max(scale.y, scale.z));
    float radius = 1.5f * maxScale;

    return RaySphereIntersection(rayOrigin, rayDirection, position, radius);
}

static float GetAxisScreenDelta(GizmoAxis axis, const XMFLOAT3& gizmoPos, float scale, float deltaX, float deltaY,
                                const XMMATRIX& view, const XMMATRIX& projection, int screenWidth, int screenHeight, float gizmoSize)
{
    XMVECTOR axisDirection = GetAxisVector(axis);
    XMVECTOR start = XMLoadFloat3(&gizmoPos);
    XMVECTOR end = start + axisDirection * gizmoSize * scale;

    XMMATRIX identity = XMMatrixIdentity();

    XMVECTOR screenStart = XMVector3Project(start, 0.0f, 0.0f,
                                            static_cast<float>(screenWidth), static_cast<float>(screenHeight),
                                            0.0f, 1.0f, projection, view, identity);

    XMVECTOR screenEnd = XMVector3Project(end, 0.0f, 0.0f,
                                          static_cast<float>(screenWidth), static_cast<float>(screenHeight),
                                          0.0f, 1.0f, projection, view, identity);

    float directionX = XMVectorGetX(screenEnd) - XMVectorGetX(screenStart);
    float directionY = XMVectorGetY(screenEnd) - XMVectorGetY(screenStart);
    float length = sqrtf(directionX * directionX + directionY * directionY);

    if (length < 0.001f) return 0.0f;

    directionX /= length;
    directionY /= length;

    return deltaX * directionX + deltaY * directionY;
}

static GizmoAxis TestRotationRingHit(const XMVECTOR& rayOrigin, const XMVECTOR& rayDirection,
                                     const XMFLOAT3& gizmoPosition, float ringRadius, float threshold)
{
    XMVECTOR center = XMLoadFloat3(&gizmoPosition);

    GizmoAxis axes[] =
    {
        GizmoAxis::X,
        GizmoAxis::Y,
        GizmoAxis::Z
    };

    GizmoAxis closestAxis = GizmoAxis::None;
    float closestDistance = FLT_MAX;

    for (GizmoAxis axis : axes)
    {
        XMVECTOR axisVector = GetAxisVector(axis);
        XMVECTOR hitPoint;

        if (!RayPlaneIntersection(rayOrigin, rayDirection, center, axisVector, hitPoint)) continue;

        float radius = XMVectorGetX(XMVector3Length(hitPoint - center));
        float distanceFromRing = fabsf(radius - ringRadius);

        if (distanceFromRing < threshold && distanceFromRing < closestDistance)
        {
            closestDistance = distanceFromRing;
            closestAxis = axis;
        }
    }

    return closestAxis;
}

static float GetRotationDelta(const XMVECTOR& rayOrigin, const XMVECTOR& previousRay,
                              const XMVECTOR& currentRay, const XMFLOAT3& position, GizmoAxis axis)
{
    XMVECTOR center = XMLoadFloat3(&position);
    XMVECTOR axisVector = GetAxisVector(axis);

    XMVECTOR previousHit;
    XMVECTOR currentHit;

    if (!RayPlaneIntersection(rayOrigin, previousRay, center, axisVector, previousHit)) return 0.0f;
    if (!RayPlaneIntersection(rayOrigin, currentRay, center, axisVector, currentHit)) return 0.0f;

    XMVECTOR previousVector = previousHit - center;
    XMVECTOR currentVector = currentHit - center;

    float previousLength = XMVectorGetX(XMVector3LengthSq(previousVector));
    float currentLength = XMVectorGetX(XMVector3LengthSq(currentVector));

    if (previousLength < 0.00001f || currentLength < 0.00001f) return 0.0f;

    previousVector = XMVector3Normalize(previousVector);
    currentVector = XMVector3Normalize(currentVector);

    float cosine = std::clamp(XMVectorGetX(XMVector3Dot(previousVector, currentVector)), -1.0f, 1.0f);

    XMVECTOR cross = XMVector3Cross(previousVector, currentVector);
    float sine = XMVectorGetX(XMVector3Dot(axisVector, cross));

    return XMConvertToDegrees(atan2f(sine, cosine));
}

Gizmo::Gizmo() = default;
Gizmo::~Gizmo() = default;

bool Gizmo::Initialize(ID3D11Device* device)
{
    if (!device) return false;

    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> psBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompile(g_GizmoVS, strlen(g_GizmoVS), "GizmoVS", nullptr, nullptr, "main", "vs_5_0",
                            D3DCOMPILE_ENABLE_STRICTNESS, 0, vsBlob.GetAddressOf(), errorBlob.GetAddressOf());

    if (FAILED(hr)) return false;

    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = D3DCompile(g_GizmoPS, strlen(g_GizmoPS), "GizmoPS", nullptr, nullptr, "main", "ps_5_0",
                    D3DCOMPILE_ENABLE_STRICTNESS, 0, psBlob.GetAddressOf(), errorBlob.GetAddressOf());

    if (FAILED(hr)) return false;

    hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());
    if (FAILED(hr)) return false;

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(),
                                   vsBlob->GetBufferSize(), m_inputLayout.GetAddressOf());

    if (FAILED(hr)) return false;

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(GizmoCB);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = device->CreateBuffer(&cbDesc, nullptr, m_constantBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.AntialiasedLineEnable = TRUE;

    hr = device->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf());
    if (FAILED(hr)) return false;

    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = FALSE;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

    hr = device->CreateDepthStencilState(&depthDesc, m_depthStencilState.GetAddressOf());
    if (FAILED(hr)) return false;

    CreateLineBuffers(device);
    return m_lineVertexBuffer != nullptr;
}

void Gizmo::CreateLineBuffers(ID3D11Device* device)
{
    std::vector<GizmoVertex> vertices;

    XMFLOAT4 red = { 1.0f, 0.15f, 0.15f, 1.0f };
    XMFLOAT4 green = { 0.25f, 1.0f, 0.25f, 1.0f };
    XMFLOAT4 blue = { 0.20f, 0.50f, 1.0f, 1.0f };
    XMFLOAT4 white = { 0.95f, 0.95f, 0.95f, 1.0f };

    auto addLine = [&](const XMFLOAT3& start, const XMFLOAT3& end, const XMFLOAT4& color)
        {
            vertices.push_back({ start, color });
            vertices.push_back({ end, color });
        };

    auto addThickLine = [&](const XMFLOAT3& start, const XMFLOAT3& end, const XMFLOAT4& color,
                            const XMFLOAT3& offset1, const XMFLOAT3& offset2)
        {
            addLine(start, end, color);

            addLine({ start.x + offset1.x, start.y + offset1.y, start.z + offset1.z },
                    { end.x + offset1.x, end.y + offset1.y, end.z + offset1.z }, color);

            addLine({ start.x - offset1.x, start.y - offset1.y, start.z - offset1.z },
                    { end.x - offset1.x, end.y - offset1.y, end.z - offset1.z }, color);

            addLine({ start.x + offset2.x, start.y + offset2.y, start.z + offset2.z },
                    { end.x + offset2.x, end.y + offset2.y, end.z + offset2.z }, color);

            addLine({ start.x - offset2.x, start.y - offset2.y, start.z - offset2.z },
                    { end.x - offset2.x, end.y - offset2.y, end.z - offset2.z }, color);
        };

    auto addWireCube = [&](const XMFLOAT3& center, float halfSize, const XMFLOAT4& color)
        {
            XMFLOAT3 points[8] =
            {
                { center.x - halfSize, center.y - halfSize, center.z - halfSize },
                { center.x + halfSize, center.y - halfSize, center.z - halfSize },
                { center.x + halfSize, center.y + halfSize, center.z - halfSize },
                { center.x - halfSize, center.y + halfSize, center.z - halfSize },
                { center.x - halfSize, center.y - halfSize, center.z + halfSize },
                { center.x + halfSize, center.y - halfSize, center.z + halfSize },
                { center.x + halfSize, center.y + halfSize, center.z + halfSize },
                { center.x - halfSize, center.y + halfSize, center.z + halfSize }
            };

            int edges[12][2] =
            {
                { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
                { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 },
                { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }
            };

            for (auto& edge : edges) addLine(points[edge[0]], points[edge[1]], color);
        };

    auto addCircle = [&](GizmoAxis axis, float radius, const XMFLOAT4& color)
        {
            for (int i = 0; i < ROTATION_SEGMENTS; ++i)
            {
                float angle0 = XM_2PI * static_cast<float>(i) / static_cast<float>(ROTATION_SEGMENTS);
                float angle1 = XM_2PI * static_cast<float>(i + 1) / static_cast<float>(ROTATION_SEGMENTS);

                float c0 = cosf(angle0);
                float s0 = sinf(angle0);
                float c1 = cosf(angle1);
                float s1 = sinf(angle1);

                XMFLOAT3 p0;
                XMFLOAT3 p1;

                if (axis == GizmoAxis::X)
                {
                    p0 = { 0.0f, c0 * radius, s0 * radius };
                    p1 = { 0.0f, c1 * radius, s1 * radius };
                }
                else if (axis == GizmoAxis::Y)
                {
                    p0 = { c0 * radius, 0.0f, s0 * radius };
                    p1 = { c1 * radius, 0.0f, s1 * radius };
                }
                else
                {
                    p0 = { c0 * radius, s0 * radius, 0.0f };
                    p1 = { c1 * radius, s1 * radius, 0.0f };
                }

                addLine(p0, p1, color);
            }
        };

    auto addThickCircle = [&](GizmoAxis axis, float radius, const XMFLOAT4& color)
        {
            addCircle(axis, radius - ROTATION_LINE_WIDTH, color);
            addCircle(axis, radius, color);
            addCircle(axis, radius + ROTATION_LINE_WIDTH, color);
        };

    float length = GIZMO_SIZE;
    float arrowSize = 0.20f;
    float thickness = 0.01f;

    g_translateXRange.Start = static_cast<UINT>(vertices.size());

    addThickLine({ 0, 0, 0 }, { length, 0, 0 }, red, { 0, thickness, 0 }, { 0, 0, thickness });
    addLine({ length, 0, 0 }, { length - arrowSize, arrowSize * 0.5f, 0 }, red);
    addLine({ length, 0, 0 }, { length - arrowSize, -arrowSize * 0.5f, 0 }, red);
    addLine({ length, 0, 0 }, { length - arrowSize, 0, arrowSize * 0.5f }, red);
    addLine({ length, 0, 0 }, { length - arrowSize, 0, -arrowSize * 0.5f }, red);

    g_translateXRange.Count = static_cast<UINT>(vertices.size()) - g_translateXRange.Start;

    g_translateYRange.Start = static_cast<UINT>(vertices.size());

    addThickLine({ 0, 0, 0 }, { 0, length, 0 }, green, { thickness, 0, 0 }, { 0, 0, thickness });
    addLine({ 0, length, 0 }, { arrowSize * 0.5f, length - arrowSize, 0 }, green);
    addLine({ 0, length, 0 }, { -arrowSize * 0.5f, length - arrowSize, 0 }, green);
    addLine({ 0, length, 0 }, { 0, length - arrowSize, arrowSize * 0.5f }, green);
    addLine({ 0, length, 0 }, { 0, length - arrowSize, -arrowSize * 0.5f }, green);

    g_translateYRange.Count = static_cast<UINT>(vertices.size()) - g_translateYRange.Start;

    g_translateZRange.Start = static_cast<UINT>(vertices.size());

    addThickLine({ 0, 0, 0 }, { 0, 0, length }, blue, { thickness, 0, 0 }, { 0, thickness, 0 });
    addLine({ 0, 0, length }, { arrowSize * 0.5f, 0, length - arrowSize }, blue);
    addLine({ 0, 0, length }, { -arrowSize * 0.5f, 0, length - arrowSize }, blue);
    addLine({ 0, 0, length }, { 0, arrowSize * 0.5f, length - arrowSize }, blue);
    addLine({ 0, 0, length }, { 0, -arrowSize * 0.5f, length - arrowSize }, blue);

    g_translateZRange.Count = static_cast<UINT>(vertices.size()) - g_translateZRange.Start;

    g_translateCenterRange.Start = static_cast<UINT>(vertices.size());
    addWireCube({ 0, 0, 0 }, 0.075f, white);
    g_translateCenterRange.Count = static_cast<UINT>(vertices.size()) - g_translateCenterRange.Start;

    g_scaleXRange.Start = static_cast<UINT>(vertices.size());
    addThickLine({ 0, 0, 0 }, { length, 0, 0 }, red, { 0, thickness, 0 }, { 0, 0, thickness });
    addWireCube({ length, 0, 0 }, 0.10f, red);
    g_scaleXRange.Count = static_cast<UINT>(vertices.size()) - g_scaleXRange.Start;

    g_scaleYRange.Start = static_cast<UINT>(vertices.size());
    addThickLine({ 0, 0, 0 }, { 0, length, 0 }, green, { thickness, 0, 0 }, { 0, 0, thickness });
    addWireCube({ 0, length, 0 }, 0.10f, green);
    g_scaleYRange.Count = static_cast<UINT>(vertices.size()) - g_scaleYRange.Start;

    g_scaleZRange.Start = static_cast<UINT>(vertices.size());
    addThickLine({ 0, 0, 0 }, { 0, 0, length }, blue, { thickness, 0, 0 }, { 0, thickness, 0 });
    addWireCube({ 0, 0, length }, 0.10f, blue);
    g_scaleZRange.Count = static_cast<UINT>(vertices.size()) - g_scaleZRange.Start;

    g_scaleCenterRange.Start = static_cast<UINT>(vertices.size());
    addWireCube({ 0, 0, 0 }, 0.09f, white);
    g_scaleCenterRange.Count = static_cast<UINT>(vertices.size()) - g_scaleCenterRange.Start;

    g_rotateXRange.Start = static_cast<UINT>(vertices.size());
    addThickCircle(GizmoAxis::X, GIZMO_SIZE, red);
    g_rotateXRange.Count = static_cast<UINT>(vertices.size()) - g_rotateXRange.Start;

    g_rotateYRange.Start = static_cast<UINT>(vertices.size());
    addThickCircle(GizmoAxis::Y, GIZMO_SIZE, green);
    g_rotateYRange.Count = static_cast<UINT>(vertices.size()) - g_rotateYRange.Start;

    g_rotateZRange.Start = static_cast<UINT>(vertices.size());
    addThickCircle(GizmoAxis::Z, GIZMO_SIZE, blue);
    g_rotateZRange.Count = static_cast<UINT>(vertices.size()) - g_rotateZRange.Start;

    g_rotateViewRange.Start = static_cast<UINT>(vertices.size());
    addThickCircle(GizmoAxis::Z, GIZMO_SIZE * 1.08f, white);
    g_rotateViewRange.Count = static_cast<UINT>(vertices.size()) - g_rotateViewRange.Start;

    m_vertexCount = static_cast<UINT>(vertices.size());

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(GizmoVertex));
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initialData = {};
    initialData.pSysMem = vertices.data();

    device->CreateBuffer(&bufferDesc, &initialData, m_lineVertexBuffer.GetAddressOf());
}

void Gizmo::Render(ID3D11DeviceContext* context, SceneObject* selectedObject, const XMMATRIX& view,
                   const XMMATRIX& projection, const XMFLOAT3& cameraPos)
{
    if (!selectedObject || m_mode == GizmoMode::None) return;

    XMFLOAT3 objectPos = selectedObject->GetTransform().GetPosition();

    float gizmoScale = GetGizmoScale(cameraPos, objectPos);
    float drawScale = m_mode == GizmoMode::Rotate ? gizmoScale * ROTATION_GIZMO_SCALE : gizmoScale;

    ID3D11RasterizerState* previousRasterizerState = nullptr;
    ID3D11DepthStencilState* previousDepthStencilState = nullptr;
    UINT previousStencilRef = 0;

    context->RSGetState(&previousRasterizerState);
    context->OMGetDepthStencilState(&previousDepthStencilState, &previousStencilRef);

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

    auto drawRange = [&](const GizmoDrawRange& range, const XMMATRIX& world, bool gray)
        {
            XMMATRIX wvp = world * view * projection;

            D3D11_MAPPED_SUBRESOURCE mapped = {};
            context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

            GizmoCB* cb = static_cast<GizmoCB*>(mapped.pData);
            XMStoreFloat4x4(&cb->WorldViewProj, XMMatrixTranspose(wvp));

            cb->OverrideColor = GIZMO_GRAY;
            cb->Settings = { gray ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f };

            context->Unmap(m_constantBuffer.Get(), 0);

            context->Draw(range.Count, range.Start);
        };

    XMMATRIX translation = XMMatrixTranslation(objectPos.x, objectPos.y, objectPos.z);
    XMMATRIX scaling = XMMatrixScaling(drawScale, drawScale, drawScale);
    XMMATRIX world = scaling * translation;

    if (m_mode == GizmoMode::Translate)
    {
        drawRange(g_translateXRange, world, m_isDragging && m_activeAxis == GizmoAxis::X);
        drawRange(g_translateYRange, world, m_isDragging && m_activeAxis == GizmoAxis::Y);
        drawRange(g_translateZRange, world, m_isDragging && m_activeAxis == GizmoAxis::Z);
        drawRange(g_translateCenterRange, world, m_isDragging && m_activeAxis == GizmoAxis::All);
    }
    else if (m_mode == GizmoMode::Scale)
    {
        drawRange(g_scaleXRange, world, m_isDragging && m_activeAxis == GizmoAxis::X);
        drawRange(g_scaleYRange, world, m_isDragging && m_activeAxis == GizmoAxis::Y);
        drawRange(g_scaleZRange, world, m_isDragging && m_activeAxis == GizmoAxis::Z);
        drawRange(g_scaleCenterRange, world, m_isDragging && m_activeAxis == GizmoAxis::All);
    }
    else if (m_mode == GizmoMode::Rotate)
    {
        drawRange(g_rotateXRange, world, m_isDragging && m_activeAxis == GizmoAxis::X);
        drawRange(g_rotateYRange, world, m_isDragging && m_activeAxis == GizmoAxis::Y);
        drawRange(g_rotateZRange, world, m_isDragging && m_activeAxis == GizmoAxis::Z);

        XMMATRIX cameraRotation = XMMatrixInverse(nullptr, view);

        cameraRotation.r[0] = XMVectorSetW(cameraRotation.r[0], 0.0f);
        cameraRotation.r[1] = XMVectorSetW(cameraRotation.r[1], 0.0f);
        cameraRotation.r[2] = XMVectorSetW(cameraRotation.r[2], 0.0f);
        cameraRotation.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

        XMMATRIX viewRingWorld = scaling * cameraRotation * translation;
        drawRange(g_rotateViewRange, viewRingWorld, false);
    }

    context->RSSetState(previousRasterizerState);
    context->OMSetDepthStencilState(previousDepthStencilState, previousStencilRef);

    if (previousRasterizerState) previousRasterizerState->Release();
    if (previousDepthStencilState) previousDepthStencilState->Release();
}

XMVECTOR Gizmo::ScreenToWorldRay(int mouseX, int mouseY, const XMMATRIX& view,
                                 const XMMATRIX& projection, int screenWidth, int screenHeight)
{
    float ndcX = (2.0f * static_cast<float>(mouseX) / static_cast<float>(screenWidth)) - 1.0f;
    float ndcY = 1.0f - (2.0f * static_cast<float>(mouseY) / static_cast<float>(screenHeight));

    XMMATRIX inverseProjection = XMMatrixInverse(nullptr, projection);
    XMMATRIX inverseView = XMMatrixInverse(nullptr, view);

    XMVECTOR rayClip = XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);
    XMVECTOR rayView = XMVector4Transform(rayClip, inverseProjection);

    rayView = XMVectorSetW(rayView, 0.0f);

    XMVECTOR rayWorld = XMVector4Transform(rayView, inverseView);
    return XMVector3Normalize(rayWorld);
}

GizmoAxis Gizmo::TestAxisHit(const XMVECTOR& rayOrigin, const XMVECTOR& rayDir,
                             const XMFLOAT3& gizmoPos, float scale)
{
    XMVECTOR gizmoCenter = XMLoadFloat3(&gizmoPos);

    float axisLength = GIZMO_SIZE * scale;
    float threshold = AXIS_HIT_THRESHOLD * scale;

    GizmoAxis axes[] =
    {
        GizmoAxis::X,
        GizmoAxis::Y,
        GizmoAxis::Z
    };

    GizmoAxis closestAxis = GizmoAxis::None;
    float closestDistance = FLT_MAX;

    for (GizmoAxis axis : axes)
    {
        XMVECTOR direction = GetAxisVector(axis);

        XMVECTOR axisStart = gizmoCenter;
        XMVECTOR axisEnd = gizmoCenter + direction * axisLength;

        XMVECTOR w0 = rayOrigin - axisStart;
        XMVECTOR u = rayDir;
        XMVECTOR v = axisEnd - axisStart;

        float a = XMVectorGetX(XMVector3Dot(u, u));
        float b = XMVectorGetX(XMVector3Dot(u, v));
        float c = XMVectorGetX(XMVector3Dot(v, v));
        float d = XMVectorGetX(XMVector3Dot(u, w0));
        float e = XMVectorGetX(XMVector3Dot(v, w0));

        float denominator = a * c - b * b;
        if (fabsf(denominator) < 0.0001f) continue;

        float rayParameter = (b * e - c * d) / denominator;
        float axisParameter = (a * e - b * d) / denominator;

        axisParameter = std::clamp(axisParameter, 0.12f, 1.1f);

        XMVECTOR closestOnRay = rayOrigin + u * rayParameter;
        XMVECTOR closestOnAxis = axisStart + v * axisParameter;

        float distance = XMVectorGetX(XMVector3Length(closestOnRay - closestOnAxis));

        if (distance < threshold && distance < closestDistance && rayParameter > 0.0f)
        {
            closestDistance = distance;
            closestAxis = axis;
        }
    }

    return closestAxis;
}

bool Gizmo::OnMouseDown(int mouseX, int mouseY, SceneObject* selectedObject, const XMMATRIX& view,
                        const XMMATRIX& projection, const XMFLOAT3& cameraPos, int screenWidth, int screenHeight, bool uniformScale)
{
    if (!selectedObject || m_mode == GizmoMode::None) return false;

    (void)uniformScale;

    XMFLOAT3 objectPos = selectedObject->GetTransform().GetPosition();
    float gizmoScale = GetGizmoScale(cameraPos, objectPos);

    XMVECTOR rayOrigin = XMLoadFloat3(&cameraPos);
    XMVECTOR rayDirection = ScreenToWorldRay(mouseX, mouseY, view, projection, screenWidth, screenHeight);

    GizmoAxis hit = GizmoAxis::None;

    if (m_mode == GizmoMode::Translate)
    {
        hit = TestAxisHit(rayOrigin, rayDirection, objectPos, gizmoScale);

        if (hit == GizmoAxis::None)
        {
            bool centerHit = IsCenterHit(mouseX, mouseY, objectPos, view, projection, screenWidth, screenHeight);
            bool objectHit = IsSelectedObjectHit(rayOrigin, rayDirection, selectedObject);

            if (centerHit || objectHit) hit = GizmoAxis::All;
        }
    }
    else if (m_mode == GizmoMode::Scale)
    {
        hit = TestAxisHit(rayOrigin, rayDirection, objectPos, gizmoScale);

        if (hit == GizmoAxis::None &&
            IsCenterHit(mouseX, mouseY, objectPos, view, projection, screenWidth, screenHeight))
        {
            hit = GizmoAxis::All;
        }
    }
    else if (m_mode == GizmoMode::Rotate)
    {
        float radius = GIZMO_SIZE * gizmoScale * ROTATION_GIZMO_SCALE;
        float threshold = ROTATION_HIT_WIDTH * gizmoScale;

        hit = TestRotationRingHit(rayOrigin, rayDirection, objectPos, radius, threshold);
    }

    if (hit == GizmoAxis::None) return false;

    m_activeAxis = hit;
    m_isDragging = true;
    m_transformModified = false;
    m_dragMode = m_mode;

    m_lastMouseX = mouseX;
    m_lastMouseY = mouseY;

    m_objectStartPos = selectedObject->GetTransform().GetPosition();
    m_objectStartRot = selectedObject->GetTransform().GetRotationEuler();
    m_objectStartScale = selectedObject->GetTransform().GetScale();

    return true;
}

void Gizmo::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY, SceneObject* selectedObject,
                        const XMMATRIX& view, const XMMATRIX& projection, const XMFLOAT3& cameraPos,
                        int screenWidth, int screenHeight, bool uniformScale)
{
    if (!m_isDragging || !selectedObject) return;

    (void)uniformScale;

    if (deltaX != 0 || deltaY != 0) m_transformModified = true;

    XMFLOAT3 position = selectedObject->GetTransform().GetPosition();
    XMFLOAT3 rotation = selectedObject->GetTransform().GetRotationEuler();
    XMFLOAT3 scale = selectedObject->GetTransform().GetScale();

    float dx = static_cast<float>(deltaX);
    float dy = static_cast<float>(deltaY);

    XMVECTOR rayOrigin = XMLoadFloat3(&cameraPos);

    if (m_dragMode == GizmoMode::Translate)
    {
        if (m_activeAxis == GizmoAxis::All)
        {
            XMVECTOR planePoint = XMLoadFloat3(&m_objectStartPos);
            XMVECTOR planeNormal = GetViewForward(view);

            XMVECTOR previousRay = ScreenToWorldRay(m_lastMouseX, m_lastMouseY,
                                                    view, projection, screenWidth, screenHeight);

            XMVECTOR currentRay = ScreenToWorldRay(mouseX, mouseY,
                                                   view, projection, screenWidth, screenHeight);

            XMVECTOR previousHit;
            XMVECTOR currentHit;

            if (RayPlaneIntersection(rayOrigin, previousRay, planePoint, planeNormal, previousHit) &&
                RayPlaneIntersection(rayOrigin, currentRay, planePoint, planeNormal, currentHit))
            {
                XMVECTOR worldDelta = currentHit - previousHit;

                XMFLOAT3 delta;
                XMStoreFloat3(&delta, worldDelta);

                position.x += delta.x;
                position.y += delta.y;
                position.z += delta.z;
            }
        }
        else
        {
            float gizmoScale = GetGizmoScale(cameraPos, position);

            XMVECTOR camera = XMLoadFloat3(&cameraPos);
            XMVECTOR object = XMLoadFloat3(&position);

            float distance = XMVectorGetX(XMVector3Length(camera - object));
            float moveSensitivity = distance * 0.002f;

            float axisDelta = GetAxisScreenDelta(m_activeAxis, position, gizmoScale, dx, dy,
                                                 view, projection, screenWidth, screenHeight, GIZMO_SIZE);

            float movement = axisDelta * moveSensitivity;

            if (m_activeAxis == GizmoAxis::X) position.x += movement;
            if (m_activeAxis == GizmoAxis::Y) position.y += movement;
            if (m_activeAxis == GizmoAxis::Z) position.z += movement;
        }

        selectedObject->GetTransform().SetPosition(position);
    }
    else if (m_dragMode == GizmoMode::Rotate)
    {
        if (m_activeAxis == GizmoAxis::X || m_activeAxis == GizmoAxis::Y || m_activeAxis == GizmoAxis::Z)
        {
            XMVECTOR previousRay = ScreenToWorldRay(m_lastMouseX, m_lastMouseY,
                                                    view, projection, screenWidth, screenHeight);

            XMVECTOR currentRay = ScreenToWorldRay(mouseX, mouseY,
                                                   view, projection, screenWidth, screenHeight);

            float angle = GetRotationDelta(rayOrigin, previousRay, currentRay, position, m_activeAxis);

            if (m_activeAxis == GizmoAxis::X) rotation.x += angle;
            if (m_activeAxis == GizmoAxis::Y) rotation.y += angle;
            if (m_activeAxis == GizmoAxis::Z) rotation.z += angle;

            selectedObject->GetTransform().SetRotationEuler(rotation);
        }
    }
    else if (m_dragMode == GizmoMode::Scale)
    {
        if (m_activeAxis == GizmoAxis::All)
        {
            float scaleDelta = (dx - dy) * 0.01f;

            scale.x = std::max(0.01f, scale.x + scaleDelta);
            scale.y = std::max(0.01f, scale.y + scaleDelta);
            scale.z = std::max(0.01f, scale.z + scaleDelta);
        }
        else
        {
            float gizmoScale = GetGizmoScale(cameraPos, position);

            float axisDelta = GetAxisScreenDelta(m_activeAxis, position, gizmoScale, dx, dy,
                                                 view, projection, screenWidth, screenHeight, GIZMO_SIZE);

            float scaleDelta = axisDelta * 0.01f;

            if (m_activeAxis == GizmoAxis::X) scale.x = std::max(0.01f, scale.x + scaleDelta);
            if (m_activeAxis == GizmoAxis::Y) scale.y = std::max(0.01f, scale.y + scaleDelta);
            if (m_activeAxis == GizmoAxis::Z) scale.z = std::max(0.01f, scale.z + scaleDelta);
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