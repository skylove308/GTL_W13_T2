
#include "ShaderRegisters.hlsl"

Texture2D InputDepth : register(t99);
Texture2D InputTexture : register(t100);

SamplerState Sampler : register(s10);

static const int KERNEL_RADIUS = 3; // 한 방향으로 몇개의 픽셀을 샘플할 지를 결정 e.g. 2인 경우 양쪽 2개씩 4개 + 중앙 픽셀 1개 = 5개
static const float Weights[KERNEL_RADIUS * 2 + 1] = {
    0.006f, 0.061f, 0.242f, 0.383f, 0.242f, 0.061f, 0.006f
};
static const float MAX_KERNEL_PIXEL_RADIUS = 4.0f;

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

cbuffer DepthOfFieldConstant : register(b1)
{
    float F_Stop;
    float SensorWidth; // mm
    float FocalDistance; // cm
    float FocalLength; // mm
};

struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

float4 CalculateBokeh(float2 UV, float CocRadius)
{
    if (CocRadius < 0.01f)
    {
        // 초점이 맞은 경우
        return InputTexture.Sample(Sampler, UV);
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
        float actualPixelOffsetMagnitude = CocRadius * MAX_KERNEL_PIXEL_RADIUS;
        float2 pixelOffset = hexKernelOffsets[i] * actualPixelOffsetMagnitude;
        float2 uvOffset = pixelOffset / TextureSize; // 픽셀 오프셋을 UV 오프셋으로 변환

        float3 sampleColor = InputTexture.Sample(Sampler, UV + uvOffset).rgb;

        float weight = 1.0f; // 단순 평균
        accumulatedColor += sampleColor * weight;
        totalWeight += weight;
    }

    if (totalWeight > 0.0f)
    {
        accumulatedColor /= totalWeight;
    }
    else
    {
        return InputTexture.Sample(Sampler, UV); // 예외 처리
    }

    // 원본 알파 값 사용
    return float4(accumulatedColor, InputTexture.Sample(Sampler, UV).a);
}

float LinearizeDepth(float NonLinearDepth, float NearClip, float FarClip)
{
    return (NearClip * FarClip) / (FarClip - NonLinearDepth * (FarClip - NearClip)); // cm
}

float CalculateCoC(float2 UV)
{
    if (FocalLength < 0.01f || FocalDistance < 0.01f || F_Stop < 0.01f)
    {
        return 0.0f;
    }
    
    const float NonLinearDepth = InputDepth.Sample(Sampler, UV).r;
    const float SceneDistance_cm = LinearizeDepth(NonLinearDepth, NearClip, FarClip);

    const float FocalLength_mm = FocalLength;
    const float FocalDistance_mm = FocalDistance * 10.0f; // cm -> mm
    const float SceneDistance_mm = SceneDistance_cm * 10.0f; // cm -> mm
    
    float Coc = 0.0f;

    if (SceneDistance_mm > 0.01f)
    {
        const float DistDiff = abs(SceneDistance_mm - FocalDistance_mm);
        const float Denominator = FocalDistance_mm * max(0.001f, SceneDistance_mm - FocalLength_mm);

        if (F_Stop > 0.0f && Denominator > 0.0001f)
        {
            /**
             * CoC 직경 (mm 단위, 센서 평면 기준)
             * 
             * 표준 CoC 공식: 
             *   CoC = (FocalLength^2 / F_Stop) * |SceneDistance - FocalDistance| / (FocalDistance * (SceneDistance - FocalLength))
             *   
             * 근사:
             *   CoC_approx = (FocalLength^2 / F_Stop) * |SceneDistance - FocalDistance| / (FocalDistance * SceneDistance)
            */
            
            Coc = (FocalLength_mm * FocalLength_mm) / F_Stop;
            Coc *= DistDiff;
            Coc /= Denominator;

            // 결과 CoC 값 스케일링
            Coc *= 0.5f;
        }
        else if (F_Stop > 0.0f && DistDiff > 0.01f)
        {
            Coc = 1.0f;
        }
    }

    return saturate(Coc);
}

float4 main(PS_Input Input) : SV_TARGET
{
    return CalculateBokeh(Input.UV, CalculateCoC(Input.UV));
}
