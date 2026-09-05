#pragma once

#include <DirectXMath.h>

namespace Engine::Graphics
{
    struct CBSkybox
    {
        DirectX::XMFLOAT4X4 InvProjection;
        DirectX::XMFLOAT4X4 InvView;

        float ExposureEV = -1.0f;
        float Intensity = 1.0f;
        DirectX::XMFLOAT2 Padding = {};
    };

    static_assert(sizeof(CBSkybox) % 16 == 0);
}