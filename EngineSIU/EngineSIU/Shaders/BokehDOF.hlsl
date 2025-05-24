
Texture2D InputTexture : register(t100);
SamplerState Sampler : register(s0);

static const int KERNEL_RADIUS = 3; // 한 방향으로 몇개의 픽셀을 샘플할 지를 결정 e.g. 2인 경우 양쪽 2개씩 4개 + 중앙 픽셀 1개 = 5개
static const float Weights[KERNEL_RADIUS * 2 + 1] = {
    0.006f, 0.061f, 0.242f, 0.383f, 0.242f, 0.061f, 0.006f
};
static const float MAX_KERNEL_PIXEL_RADIUS = 16.0f;

static const int NUM_HEX_SAMPLES = 7;
static const float2 hexKernelOffsets[NUM_HEX_SAMPLES] = {
    float2(0.0f, 0.0f),      // Center
    float2(1.0f, 0.0f),      // Right
    float2(0.5f, 0.866f),   // Top-Right (cos(60), sin(60))
    float2(-0.5f, 0.866f),  // Top-Left
    float2(-1.0f, 0.0f),     // Left
    float2(-0.5f, -0.866f), // Bottom-Left
    float2(0.5f, -0.866f)   // Bottom-Right
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

float4 CalculateBokeh(float2 uv, float cocRadius)
{
    if (cocRadius < 0.01f)
    { // 초점이 맞은 경우
        return InputTexture.Sample(Sampler, uv);
    }

    float3 accumulatedColor = float3(0.0f, 0.0f, 0.0f);
    float totalWeight = 0.0f;

    // 커널 오프셋 (GLSL 예시와 유사하게)
    // 실제로는 더 많은 샘플과 정교한 분포를 사용
    static const float2 kernelOffsets[5] = {
        float2(0.0f, 0.0f),
        float2(0.7f, 0.0f),
        float2(-0.7f, 0.0f),
        float2(0.0f, 0.7f),
        float2(0.0f, -0.7f)
    };

    for (int i = 0; i < NUM_HEX_SAMPLES; ++i)
    {
        // cocRadius: 0 (선명) ~ 1 (최대 흐림) 사이의 정규화된 값이라고 가정
        // 또는 실제 픽셀 단위의 흐림 반경일 수 있음.
        // 여기서는 정규화된 값으로 가정하고 MAX_KERNEL_PIXEL_RADIUS와 곱하여 실제 픽셀 오프셋을 만듦
        float actualPixelOffsetMagnitude = cocRadius * MAX_KERNEL_PIXEL_RADIUS;
        float2 pixelOffset = hexKernelOffsets[i] * actualPixelOffsetMagnitude;
        float2 uvOffset = pixelOffset / TextureSize; // 픽셀 오프셋을 UV 오프셋으로 변환

        float3 sampleColor = InputTexture.Sample(Sampler, uv + uvOffset).rgb;

        float weight = 1.0f; // 단순 평균
        accumulatedColor += sampleColor * weight;
        totalWeight += weight;
    }

    if (totalWeight > 0.0f)
    {
        accumulatedColor /= totalWeight;
    } else
    {
        return InputTexture.Sample(Sampler, uv); // 예외 처리
    }

    // 원본 알파 값 사용
    return float4(accumulatedColor, InputTexture.Sample(Sampler, uv).a);
}

float4 main(PS_Input Input) : SV_TARGET
{
    return CalculateBokeh(Input.UV, 0.5);
}
