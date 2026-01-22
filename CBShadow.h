#include "Renderer.h"
#include <DirectXMath.h>

using namespace DirectX;

struct CBShadow
{
    XMFLOAT4X4 LightViewProj[NUM_CASCADES];
    XMFLOAT4 CascadeSplits; // xyz = split depths
};
