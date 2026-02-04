#pragma once
#include "Light.h"
#include <DirectXMath.h>
#define MAX_LIGHTS 8

using namespace DirectX;

struct alignas(16) CBLight
{
	int LightCount;
	XMFLOAT3 CameraPosition;
	Light Lights[MAX_LIGHTS];
};
