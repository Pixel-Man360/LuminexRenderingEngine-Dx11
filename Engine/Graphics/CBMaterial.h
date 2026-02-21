#pragma once
#include <DirectXMath.h>

using namespace DirectX;

struct alignas(16) CBMaterial
{
    XMFLOAT4 Albedo;           // RGB albedo + alpha
    float Metallic;
    float Roughness;
    float AO;
    float Padding1;

    XMFLOAT2 Tiling;          
    XMFLOAT2 Offset;

    // Flags for which texture maps are bound (0 = use scalar, 1 = use texture)
    uint32_t UseAlbedoMap;
    uint32_t UseNormalMap;
    uint32_t UseMetallicMap;
    uint32_t UseRoughnessMap;
    uint32_t UseAOMap;
    uint32_t Padding2[3];
};
