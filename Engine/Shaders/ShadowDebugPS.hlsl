Texture2DArray ShadowMapArray : register(t0);
SamplerState Sampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    // Display all 4 cascades in a 2x2 grid
    float2 uv = input.uv;
    int cascadeIndex = 0;
    
    // Determine which quadrant we're in (top-left=0, top-right=1, bottom-left=2, bottom-right=3)
    if (uv.x >= 0.5f)
    {
        cascadeIndex += 1;
        uv.x = (uv.x - 0.5f) * 2.0f;
    }
    else
    {
        uv.x = uv.x * 2.0f;
    }
    
    if (uv.y >= 0.5f)
    {
        cascadeIndex += 2;
        uv.y = (uv.y - 0.5f) * 2.0f;
    }
    else
    {
        uv.y = uv.y * 2.0f;
    }
    
    float depth = ShadowMapArray.Sample(Sampler, float3(uv, cascadeIndex)).r;
    
    // Enhance contrast - depth values close to 1 are far, close to 0 are near
    // Apply power curve for better visualization
    float visualDepth = pow(depth, 0.4f);
    
    return float4(visualDepth, visualDepth, visualDepth, 1.0f);
}