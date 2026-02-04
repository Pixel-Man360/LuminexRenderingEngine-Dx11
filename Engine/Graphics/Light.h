#pragma once
#include <DirectXMath.h>

using namespace DirectX;

enum LightType
{
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT = 1
};

struct alignas(16) Light
{
    int Type = LIGHT_DIRECTIONAL;
    XMFLOAT3 Color = { 1,1,1 };

    XMFLOAT3 Direction = { 0,0,0 };
    float Range = 1.0f;

    XMFLOAT3 Position = { 0,0,0 };
    float Intensity = 1.0f;
};
