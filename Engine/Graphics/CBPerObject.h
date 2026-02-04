#pragma once
#include <DirectXMath.h>

using namespace DirectX;

struct alignas(16) CBPerObject
{
    XMFLOAT4X4 World;
    XMFLOAT4X4 WorldInvTranspose;
    XMFLOAT4X4 View;
    XMFLOAT4X4 Projection;
	XMFLOAT4X4 LightViewProj;
};
