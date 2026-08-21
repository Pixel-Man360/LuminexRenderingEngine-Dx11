cbuffer CBPerObject : register(b0)
{
    float4x4 World;
    float4x4 WorldInvTranspose;
    float4x4 View;
    float4x4 Projection;
    float4x4 LightViewProj;
    float4 SelectionColor;
};

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 tangent : TANGENT;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 normalWS : NORMAL;
    float3 posWS : POSITION;
    float2 uv : TEXCOORD0;
    float4 posVS : TEXCOORD1;
    float4 selectionColor : TEXCOORD2;
    float4 tangentWS : TEXCOORD3;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    float4 posWorld = mul(float4(input.position, 1.0f), World);

    output.posWS = posWorld.xyz;

    float4 posView = mul(posWorld, View);

    output.posVS = posView;
    output.position = mul(posView, Projection);

    float3 N = normalize(mul(input.normal, (float3x3)WorldInvTranspose));

    float3 tangentOS = input.tangent.xyz;

    if (dot(tangentOS, tangentOS) < 0.000001f)
    {
        float3 referenceAxis = abs(input.normal.y) < 0.999f ? float3(0.0f, 1.0f, 0.0f) : float3(1.0f, 0.0f, 0.0f);
        tangentOS = normalize(cross(referenceAxis, input.normal));
    }

    float3 T = normalize(mul(tangentOS, (float3x3)World));
    T = normalize(T - N * dot(N, T));

    output.normalWS = N;
    output.tangentWS = float4(T, input.tangent.w);
    output.uv = input.uv;
    output.selectionColor = SelectionColor;

    return output;
}