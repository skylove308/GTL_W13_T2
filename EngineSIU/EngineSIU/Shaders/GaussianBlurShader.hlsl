
Texture2D InputTexture : register(t100);
SamplerState Sampler : register(s0);

static const int KERNEL_RADIUS = 3; // 한 방향으로 몇개의 픽셀을 샘플할 지를 결정 e.g. 2인 경우 양쪽 2개씩 4개 + 중앙 픽셀 1개 = 5개
static const float Weights[KERNEL_RADIUS * 2 + 1] = {
    0.006f, 0.061f, 0.242f, 0.383f, 0.242f, 0.061f, 0.006f
};

cbuffer TextureSize : register(b0)
{
    float2 TextureSize;
    float2 Padding;
}

struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

PS_Input mainVS(uint VertexID : SV_VertexID)
{
    PS_Input Output;

    float2 QuadPositions[4] = {
        float2(-1, -1),
        float2( 1, -1),
        float2(-1,  1),
        float2( 1,  1)
    };

    float2 UVs[4] = {
        float2(0, 1),
        float2(1, 1),
        float2(0, 0),
        float2(1, 0)
    };

    uint Indices[6] = {
        0, 2, 1,
        1, 2, 3
    };

    uint Index = Indices[VertexID];
    Output.Position = float4(QuadPositions[Index], 0, 1);
    Output.UV = UVs[Index];

    return Output;
}

float4 mainPS(PS_Input Input) : SV_TARGET
{
    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = -KERNEL_RADIUS; i <= KERNEL_RADIUS; ++i)
    {
        float2 offset = float2(i / TextureSize.x, 0.0f);
        finalColor += InputTexture.Sample(Sampler, Input.UV + offset) * Weights[i + KERNEL_RADIUS];
    }
    return finalColor;
}
