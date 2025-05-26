
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

float4 main(PS_Input Input) : SV_TARGET
{
    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = -KERNEL_RADIUS; i <= KERNEL_RADIUS; ++i)
    {
        float2 offset = float2(i / TextureSize.x, 0.0f);
        finalColor += InputTexture.Sample(Sampler, Input.UV + offset) * Weights[i + KERNEL_RADIUS];
    }
    return finalColor;
}
