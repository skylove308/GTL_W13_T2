
#include "ShaderRegisters.hlsl"

Texture2D InputDepth : register(t99);
Texture2D InputTexture : register(t100);
Texture2D BlurTexture : register(t102);

SamplerState Sampler : register(s10);

static const float MAX_KERNEL_PIXEL_RADIUS = 8.0f;

// 포아송 디스크 샘플링 패턴 (19개 샘플)
static const int NUM_SAMPLES = 19;
static const float2 PoissonSamples[NUM_SAMPLES] = {
    float2(0.0f, 0.0f),        // Center
    float2(0.527837f, -0.085868f),
    float2(-0.040088f, 0.536087f),
    float2(-0.670445f, -0.179949f),
    float2(-0.419418f, -0.616039f),
    float2(0.440453f, -0.639399f),
    float2(-0.757088f, 0.349334f),
    float2(0.574619f, 0.685879f),
    float2(0.976331f, 0.15346f),
    float2(-0.624817f, 0.765323f),
    float2(0.122747f, 0.970479f),
    float2(0.840895f, -0.52498f),
    float2(-0.043655f, -0.967251f),
    float2(-0.848312f, -0.519516f),
    float2(-0.998088f, 0.054414f),
    float2(0.285328f, 0.418364f),
    float2(-0.273026f, -0.340141f),
    float2(0.725791f, 0.326734f),
    float2(-0.311553f, 0.148081f)
};

cbuffer TextureSize : register(b0)
{
    float2 TextureSize;
    float2 Padding;
}

cbuffer DepthOfFieldConstant : register(b1)
{
    float F_Stop;
    float SensorWidth_mm; // mm
    float FocalDistance_World; // cm
    float FocalLength_mm; // mm
};

struct PS_Input
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

float LinearizeDepth(float NonLinearDepth, float Near, float Far)
{
    return (Near * Far) / (max(0.0001f, Far - NonLinearDepth * (Far - Near)));
}

float4 CalculateBokeh(float2 UV, float CocRadius)
{
    if (CocRadius < 0.01f)
    {
        // 초점이 맞은 경우
        return InputTexture.Sample(Sampler, UV);
    }

    float3 AccumulatedColor = float3(0.0f, 0.0f, 0.0f);
    float TotalWeight = 0.0f;

    // 가우시안 시그마 값 계산
    float Sigma = CocRadius * 0.5f;
    float TwoSigmaSquared = 2.0f * Sigma * Sigma;

    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        // cocRadius: 0 (선명) ~ 1 (최대 흐림) 사이의 정규화된 값이라고 가정
        // 또는 실제 픽셀 단위의 흐림 반경일 수 있음.
        // 여기서는 정규화된 값으로 가정하고 MAX_KERNEL_PIXEL_RADIUS와 곱하여 실제 픽셀 오프셋을 만듦
        float actualPixelOffsetMagnitude = CocRadius * MAX_KERNEL_PIXEL_RADIUS;
        float2 pixelOffset = PoissonSamples[i] * actualPixelOffsetMagnitude;
        float2 uvOffset = pixelOffset / TextureSize; // 픽셀 오프셋을 UV 오프셋으로 변환

        float3 sampleColor = InputTexture.Sample(Sampler, UV + uvOffset).rgb;

        // 거리 기반 가우시안 가중치 계산
        float distance = length(PoissonSamples[i]);
        float weight = exp(-(distance * distance) / TwoSigmaSquared);
        
        // 중앙 샘플에 더 높은 가중치 부여
        if (i == 0) {
            weight *= 2.0f;
        }

        AccumulatedColor += sampleColor * weight;
        TotalWeight += weight;
    }

    if (TotalWeight > 0.0f)
    {
        AccumulatedColor /= TotalWeight;
    }
    else
    {
        return InputTexture.Sample(Sampler, UV);
    }

    return float4(AccumulatedColor, InputTexture.Sample(Sampler, UV).a);
}

float CalculateCoC(float2 UV)
{
    if (FocalLength_mm < 0.01f || FocalDistance_World < 0.01f || F_Stop < 0.01f)
    {
        return 0.0f;
    }
    
    const float NonLinearDepth = InputDepth.Sample(Sampler, UV).r;
    const float SceneDistance_cm = LinearizeDepth(NonLinearDepth, NearClip, FarClip);

    const float FocalDistance_mm = FocalDistance_World * 10.0f; // cm -> mm
    const float SceneDistance_mm = SceneDistance_cm * 10.0f; // cm -> mm
    
    float Coc = 0.0f;

    if (SceneDistance_mm > 0.01f)
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
        
        const float DistDiff = abs(SceneDistance_mm - FocalDistance_mm);
        //const float Denominator = FocalDistance_mm * max(0.001f, SceneDistance_mm - FocalLength_mm);
        const float Denominator = FocalDistance_mm * max(0.001f, SceneDistance_mm);

        if (F_Stop > 0.0f && Denominator > 0.0001f)
        {
            float CoC_sensor = (FocalLength_mm * FocalLength_mm / F_Stop) * DistDiff / Denominator;

            // 센서 크기로 정규화
            Coc = CoC_sensor / SensorWidth_mm;
            //Coc = CoC_sensor;
            
            // 결과 CoC 값 스케일링
            Coc *= 2.0f;
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

float4 main_Composite(PS_Input Input) : SV_TARGET
{
    float4 SceneColor = InputTexture.Sample(Sampler, Input.UV);
    float4 BlurColor = BlurTexture.Sample(Sampler, Input.UV);

    float Coc = CalculateCoC(Input.UV);
    //return float4(Coc, Coc, Coc, 1.0f);

    float3 FinalColor = lerp(SceneColor.rgb, BlurColor.rgb, Coc);
    return float4(FinalColor, SceneColor.a);
}
