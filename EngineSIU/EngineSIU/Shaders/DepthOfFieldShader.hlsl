
#include "ShaderRegisters.hlsl"

Texture2D LayerInfoTexture : register(t90);
Texture2D BlurredNearTexture : register(t91);
Texture2D BlurredFarTexture : register(t92);
Texture2D FilteredCoCTexture : register(t93);
Texture2D InputDepth : register(t99);
Texture2D SceneTexture : register(t100);

SamplerState LinearSampler : register(s10);
SamplerState PointSampler : register(s11);

static const float MAX_KERNEL_PIXEL_RADIUS = 5.0f;

// 포아송 디스크 샘플링 패턴 (19개 샘플)
static const int NUM_SAMPLES = 19;
static const float2 PoissonSamples[NUM_SAMPLES] = {
    float2(0.0f, 0.0f),        // Center
    float2(0.527837f, -0.085868f), float2(-0.040088f, 0.536087f), float2(-0.670445f, -0.179949f),
    float2(-0.419418f, -0.616039f), float2(0.440453f, -0.639399f), float2(-0.757088f, 0.349334f),
    float2(0.574619f, 0.685879f), float2(0.976331f, 0.15346f), float2(-0.624817f, 0.765323f),
    float2(0.122747f, 0.970479f), float2(0.840895f, -0.52498f), float2(-0.043655f, -0.967251f),
    float2(-0.848312f, -0.519516f), float2(-0.998088f, 0.054414f), float2(0.285328f, 0.418364f),
    float2(-0.273026f, -0.340141f), float2(0.725791f, 0.326734f), float2(-0.311553f, 0.148081f)
};

cbuffer TextureSize : register(b0)
{
    float2 TexturePixelSize;
    float BokehIntensityScale;
    float TextureSizePadding;
}

cbuffer DepthOfFieldConstant : register(b1)
{
    float F_Stop;
    float SensorWidth_mm; // mm
    float FocalDistance_World; // cm
    float FocalLength_mm; // mm

    float CoCScaleFactor;
    float InFocusThreshold; // [0, 1]
    float DOFPadding1;
    float DOFPadding2;
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

float4 CalculateBlur(float2 TexUV, float2 CurrentTextureSize)
{
    float4 CenterPixelData = SceneTexture.Sample(LinearSampler, TexUV);
    float3 CenterColor = CenterPixelData.rgb;
    float CenterCoC = CenterPixelData.a;

    float3 AccumulatedColor = float3(0.0f, 0.0f, 0.0f);
    float TotalWeight = 0.0f;

    // 실제 블러 반경 (픽셀 단위)
    float ActualPixelRadius = CenterCoC * MAX_KERNEL_PIXEL_RADIUS * BokehIntensityScale;

    float NormalizedSigma = 0.5f;
    float TwoNormalizedSigmaSquared = max(0.0001f, 2.0f * NormalizedSigma * NormalizedSigma);

    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        // 포아송 샘플 오프셋에 실제 픽셀 반경을 곱함
        float2 PixelOffset = PoissonSamples[i] * ActualPixelRadius;
        float2 UVOffset = PixelOffset / CurrentTextureSize; // 픽셀 오프셋을 UV 오프셋으로 변환
        float2 SampleUV = TexUV + UVOffset;

        // 샘플링할 픽셀의 데이터 (색상 및 해당 픽셀의 CoC)
        float4 SampleData = SceneTexture.Sample(LinearSampler, SampleUV);
        float3 SampleColor = SampleData.rgb;
        float SampleCoC = SampleData.a; // 샘플링된 픽셀의 CoC 값

        float Weight = 1.0f;

        // 거리 기반 가우시안 가중치 (정규화된 거리 사용)
        float DistanceSquared = dot(PoissonSamples[i], PoissonSamples[i]);
        float GaussianWeight = exp(-DistanceSquared / TwoNormalizedSigmaSquared);
        Weight *= GaussianWeight;

        float HighlightBoost = 1.0f + saturate(dot(SampleColor, SampleColor) - 0.8f) * 5.0f;
        Weight *= HighlightBoost;

        AccumulatedColor += SampleColor * Weight;
        TotalWeight += Weight;

        if (CenterCoC > 0.01f) // CenterCoC가 유의미할 때
        {
            float Ratio = (SampleCoC + 0.01f) / (CenterCoC + 0.01f);
            Weight *= smoothstep(0.1f, 0.5f, Ratio);
        }
        else
        {
            Weight *= smoothstep(0.2f, 0.0f, SampleCoC);
        }


        if (Weight > 0.0001f)
        {
            AccumulatedColor += SampleColor * Weight;
            TotalWeight += Weight;
        }
    }

    if (TotalWeight > 0.001f) // 분모 0 방지
    {
        AccumulatedColor /= TotalWeight;
    }
    else
    {
        // 샘플을 거의 못 찾았거나 모든 가중치가 0에 가까우면 원본 색상 반환
        return CenterPixelData;
    }

    return float4(AccumulatedColor, CenterCoC);
}

float4 CalculateNearBlur(float2 TexUV, float2 CurrentTextureSize)
{
    float4 CenterPixelData = SceneTexture.Sample(LinearSampler, TexUV);
    float3 CenterColor = CenterPixelData.rgb;
    float CenterCoC = CenterPixelData.a;

    if (CenterCoC < 0.001f)
    {
        return CenterPixelData;
    }

    float3 AccumulatedColor = float3(0.0f, 0.0f, 0.0f);
    float TotalWeight = 0.0f;
    float MaxCoC = CenterCoC; // 최대 CoC 추적

    // 확장된 블러 반경 - 주변 픽셀들의 CoC도 고려
    float ActualPixelRadius = CenterCoC * MAX_KERNEL_PIXEL_RADIUS * BokehIntensityScale;

    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        // 1차: 기본 반경으로 샘플링
        float2 PixelOffset = PoissonSamples[i] * ActualPixelRadius;
        float2 UVOffset = PixelOffset / CurrentTextureSize;
        float2 SampleUV = TexUV + UVOffset;

        float4 SampleData = SceneTexture.Sample(LinearSampler, SampleUV);
        float3 SampleColor = SampleData.rgb;
        float SampleCoC = SampleData.a;
        
        float Weight = 1.0f;

        // 샘플 픽셀의 CoC가 중심보다 클 경우, 그 영향 반경 계산
        float SampleInfluenceRadius = SampleCoC * MAX_KERNEL_PIXEL_RADIUS * BokehIntensityScale;
        // 현재 샘플 위치에서 중심까지의 거리
        float DistanceToCenter = length(PixelOffset);

        // Near 레이어 전용 가중치 계산
        // 1. 샘플 픽셀이 중심에 영향을 줄 수 있는지 확인
        if (DistanceToCenter <= SampleInfluenceRadius && SampleInfluenceRadius > 0.001f)
        {
            // 샘플 픽셀의 영향권 내에 있으면 높은 가중치
            Weight *= saturate(1.0f - (DistanceToCenter / SampleInfluenceRadius));
        }
        else
        {
            // 기본 가우시안 가중치
            float Sigma = 0.5f; // 2.0f * 0.5f * 0.5f = 0.5f
            float NormalizedKernelDistance = length(PoissonSamples[i]);
            Weight *= exp(-NormalizedKernelDistance * NormalizedKernelDistance / Sigma);
        }

        // 하이라이트 부스트
        float HighlightBoost = 1.0f + saturate(dot(SampleColor, SampleColor) - 0.8f) * 5.0f;
        Weight *= HighlightBoost;

        // CoC 기반 가중치 (Near 레이어 전용)
        // 샘플의 CoC가 클수록 더 많이 기여하도록
        Weight *= saturate(SampleCoC * 2.0f + 0.1f);

        if (Weight > 0.0001f)
        {
            AccumulatedColor += SampleColor * Weight;
            TotalWeight += Weight;
            MaxCoC = max(MaxCoC, SampleCoC);
        }
    }

    if (TotalWeight > 0.001f)
    {
        AccumulatedColor /= TotalWeight;
    }
    else
    {
        return CenterPixelData;
    }

    return float4(AccumulatedColor, MaxCoC);
}

float4 CalculateFarBlur(float2 TexUV, float2 CurrentTextureSize)
{
    float4 CenterPixelData = SceneTexture.Sample(LinearSampler, TexUV);
    float3 CenterColor = CenterPixelData.rgb;
    float CenterCoC = CenterPixelData.a;

    if (CenterCoC < 0.001f)
    {
        return CenterPixelData;
    }

    float3 AccumulatedColor = float3(0.0f, 0.0f, 0.0f);
    float TotalWeight = 0.0f;

    float ActualPixelRadius = CenterCoC * MAX_KERNEL_PIXEL_RADIUS * BokehIntensityScale;

    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        float2 PixelOffset = PoissonSamples[i] * ActualPixelRadius;
        float2 UVOffset = PixelOffset / CurrentTextureSize;
        float2 SampleUV = TexUV + UVOffset;

        float4 SampleData = SceneTexture.Sample(LinearSampler, SampleUV);
        float3 SampleColor = SampleData.rgb;
        float SampleCoC = SampleData.a;

        float Weight = 1.0f;

        // 거리 기반 가우시안 가중치
        float Sigma = 0.5f; // 2.0f * 0.5f * 0.5f = 0.5f
        float NormalizedKernelDistanceSq = dot(PoissonSamples[i], PoissonSamples[i]);
        float GaussianWeight = exp(-NormalizedKernelDistanceSq / Sigma);
        Weight *= GaussianWeight;

        // 하이라이트 부스트
        float HighlightBoost = 1.0f + saturate(dot(SampleColor, SampleColor) - 0.8f) * 5.0f;
        Weight *= HighlightBoost;

        // Far 레이어의 경우 기존 로직 유지 (배경 누출 방지)
        if (CenterCoC > 0.01f)
        {
            float Ratio = (SampleCoC + 0.01f) / (CenterCoC + 0.01f);
            Weight *= smoothstep(0.1f, 0.5f, Ratio);
        }
        else
        {
            Weight *= smoothstep(0.2f, 0.0f, SampleCoC);
        }

        if (Weight > 0.0001f)
        {
            AccumulatedColor += SampleColor * Weight;
            TotalWeight += Weight;
        }
    }

    if (TotalWeight > 0.001f)
    {
        AccumulatedColor /= TotalWeight;
    }
    else
    {
        return CenterPixelData;
    }

    return float4(AccumulatedColor, CenterCoC);
}

float4 CalculateLayerCoCAndMasks(float2 UV)
{
    float4 LayerInfo = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // 유효성 검사
    if (FocalLength_mm < 0.01f || FocalDistance_World < 0.01f || F_Stop < 0.01f ||
        SensorWidth_mm < 0.01f || NearClip <= 0.0f || FarClip <= NearClip || CoCScaleFactor < 0.0f)
    {
        LayerInfo.z = 1.0f; // 문제가 있을 경우, 기본적으로 모든 것을 초점 맞음으로 처리
        return LayerInfo;
    }
    
    const float NonLinearDepth = InputDepth.Sample(PointSampler, UV).r;
    // LinearizeDepth에 전달하는 NearClip, FarClip은 뎁스 버퍼가 생성될 때 사용된 카메라의 값과 일치해야 합니다.
    // 또한, FocalDistance_World와 단위가 같아야 합니다.
    const float SceneDistance_World = LinearizeDepth(NonLinearDepth, NearClip, FarClip);

    // 모든 단위를 mm로 통일
    const float FocalDistance_mm = FocalDistance_World * 10.0f; // cm -> mm (월드 단위가 cm라고 가정)
    const float SceneDistance_mm = SceneDistance_World * 10.0f; // cm -> mm (월드 단위가 cm라고 가정)
    
    float ScaledSignedCoc = 0.0f; // 최종적으로 스케일링된 부호 있는 CoC

    // SceneDistance_mm가 유효한 값일 때만 계산 (예: 매우 큰 값 방지)
    // FarClip을 초과하는 거리는 FarClip에서의 CoC로 clamp하거나, 다른 처리가 필요할 수 있음.
    // 현재는 LinearizeDepth 결과 그대로 사용.
    if (SceneDistance_mm > 0.001f) // 유효한 거리인지 확인
    {
        // (SceneDistance_mm - FocalLength_mm)가 0 또는 음수가 되는 것을 방지 (특히 근거리 물체)
        // 실제 물리 기반 공식 대신 근사 공식을 사용하는 경우가 많음.
        // 기존 코드의 근사 분모: FocalDistance_mm * max(0.001f, SceneDistance_mm)
        const float Denominator = FocalDistance_mm * max(0.001f, SceneDistance_mm);
        const float SignedDistanceDiff_mm = SceneDistance_mm - FocalDistance_mm;

        if (abs(Denominator) > 0.00001f) // 분모가 0에 매우 가깝지 않은지 확인
        {
            // CoC 직경 (mm 단위, 센서 평면 기준), 부호 유지
            // CoC_sensor_signed = (FocalLength^2 / F_Stop) * (SceneDistance - FocalDistance) / Denominator
            float CoC_sensor_signed = (FocalLength_mm * FocalLength_mm / F_Stop) * SignedDistanceDiff_mm / Denominator;

            // 센서 크기로 정규화 (부호 유지)
            float NormalizedSignedCoc = CoC_sensor_signed / SensorWidth_mm;
            
            // 제공된 스케일 팩터 적용
            ScaledSignedCoc = NormalizedSignedCoc * CoCScaleFactor;
        }
        else if (abs(SignedDistanceDiff_mm) > 0.01f) // 분모 문제 발생 시, 거리 차이가 있다면 최대 CoC 부여
        {
            // CoCScaleFactor가 양수라고 가정
            ScaledSignedCoc = (SignedDistanceDiff_mm > 0 ? 1.0f : -1.0f) * CoCScaleFactor; // 부호 있는 최대 스케일 CoC
        }
        // 그 외의 경우는 ScaledSignedCoc = 0.0f 유지
    }
    // SceneDistance_mm <= 0.001f (예: 너무 가깝거나 잘못된 뎁스)인 경우 ScaledSignedCoc = 0.0f 유지

    LayerInfo.w = saturate(ScaledSignedCoc * 0.5f + 0.5f); // Raw Scaled Signed CoC 저장

    float CocMagnitude = abs(ScaledSignedCoc);
    float SaturatedCocMagnitude = saturate(CocMagnitude); // 블러 강도로 사용될 값 (0~1)

    if (SaturatedCocMagnitude < InFocusThreshold)
    {
        LayerInfo.z = 1.0f; // In-focus
        // LayerInfo.x (FarCoC) and LayerInfo.y (NearCoC) remain 0.0
    }
    else
    {
        LayerInfo.z = 0.0f; // Out-of-focus
        if (ScaledSignedCoc > 0.0f) // Far field (SceneDistance > FocalDistance)
        {
            LayerInfo.x = SaturatedCocMagnitude; // Far CoC
        }
        else // Near field (ScaledSignedCoc < 0.0f, SceneDistance < FocalDistance)
        {
            LayerInfo.y = SaturatedCocMagnitude; // Near CoC
        }
    }
    
    return LayerInfo;
}

float4 PS_GenerateLayer(PS_Input Input) : SV_TARGET
{
    /**
     * R: Far Field Coc
     * G: Near Field Coc
     * B: In-focus Mask
     * A: Raw Signed CoC
     */
    return CalculateLayerCoCAndMasks(Input.UV);
}

float4 PS_FilterNearCoC_Max(PS_Input Input) : SV_TARGET
{
    float2 UV = Input.UV;
    float2 TexelSize = TexturePixelSize; 

    float MaxNearCoC = 0.0f;

    // 3x3 Max Filter 예시 (커널 크기는 필요에 따라 조절)
    // 좀 더 넓은 영역을 원하면 5x5 등으로 확장 가능
    const int KernelRadius = 3; // 3x3 ( -1, 0, 1 )
    for (int y = -KernelRadius; y <= KernelRadius; ++y)
    {
        for (int x = -KernelRadius; x <= KernelRadius; ++x)
        {
            float2 Offset = float2(x, y) * TexelSize;
            // LayerInfoTexture 샘플링 시 PointSampler 사용 권장
            float CurrentNearCoC = LayerInfoTexture.Sample(PointSampler, UV + Offset).g; 
            MaxNearCoC = max(MaxNearCoC, CurrentNearCoC);
        }
    }
    
    // 필터링된 MaxNearCoC 값을 예를 들어 R 채널에 저장합니다.
    // 다른 채널 값은 필요에 따라 LayerInfoTexture에서 가져오거나 0으로 설정할 수 있습니다.
    // 여기서는 다른 채널은 사용하지 않는다고 가정하고 MaxNearCoC만 R 채널에 씁니다.
    return float4(MaxNearCoC, MaxNearCoC, MaxNearCoC, MaxNearCoC);
}

float4 PS_BlurLayer(PS_Input Input) : SV_TARGET
{
    float2 DownSampleTextureSize = 1.0 / TexturePixelSize;
    return CalculateBlur(Input.UV, DownSampleTextureSize);
}


float4 PS_BlurNearLayer(PS_Input Input) : SV_TARGET
{
    float2 DownSampleTextureSize = 1.0 / TexturePixelSize; // 실제 텍스처 크기
    return CalculateNearBlur(Input.UV, DownSampleTextureSize);
}

float4 PS_BlurFarLayer(PS_Input Input) : SV_TARGET
{
    float2 DownSampleTextureSize = 1.0 / TexturePixelSize; // 실제 텍스처 크기
    return CalculateFarBlur(Input.UV, DownSampleTextureSize);
}

float4 PS_ExtractAndDownsampleLayer(PS_Input Input) : SV_TARGET
{
    float2 UV = Input.UV;
    float2 TexelSize = TexturePixelSize;

    // 2x2 샘플링 오프셋 (풀 해상도 텍셀 기준)
    float2 Offsets[4] =
    {
        float2(-0.5f * TexelSize.x, -0.5f * TexelSize.y),
        float2( 0.5f * TexelSize.x, -0.5f * TexelSize.y),
        float2(-0.5f * TexelSize.x,  0.5f * TexelSize.y),
        float2( 0.5f * TexelSize.x,  0.5f * TexelSize.y)
    };

    float3 AccumulatedColor = float3(0.0f, 0.0f, 0.0f);
    float TotalWeight = 0.0f;
    float RepresentativeCoC = 0.0f; // 합성 시 사용할 CoC 대표값 (예: 최대값)

    for (int i = 0; i < 4; ++i)
    {
        float2 SampleUV = UV + Offsets[i];

        float CurrentCoC = 
#ifdef NEAR
            FilteredCoCTexture.Sample(LinearSampler, SampleUV).r;
#else
            LayerInfoTexture.Sample(LinearSampler, SampleUV).r;
#endif

        float4 CurrentSceneColor = SceneTexture.Sample(LinearSampler, SampleUV);
        AccumulatedColor += CurrentSceneColor.rgb; // CoC를 가중치로 사용
        TotalWeight += 1.0;
        RepresentativeCoC = max(RepresentativeCoC, CurrentCoC);
    }

    if (TotalWeight > 0.001f)
    {
        return float4(AccumulatedColor / TotalWeight, RepresentativeCoC); // 또는 totalNearWeight / 4.0f 등
    }
    
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 DebugCoC(float2 UV)
{
    float4 layerInfo = LayerInfoTexture.Sample(LinearSampler, UV); // 또는 LinearClampSampler
    
    float farCoC = layerInfo.r;        // 0.0 ~ 1.0 (원경 흐림 강도)
    float nearCoC = layerInfo.g;       // 0.0 ~ 1.0 (근경 흐림 강도)
    float inFocusMask = layerInfo.b;   // 0.0 또는 1.0 (초점 마스크)
    float mappedSignedCoC = layerInfo.a; // 0.0 ~ 1.0 (부호있는 CoC를 0-1로 매핑한 값, 0.5가 초점 근처)

    // --- 디버깅 출력 선택 ---

    // 예시 1: Far CoC 값을 회색조로 표시
    // return float4(farCoC, farCoC, farCoC, 1.0f);

    // 예시 2: Near CoC 값을 회색조로 표시
    // return float4(nearCoC, nearCoC, nearCoC, 1.0f);

    // 예시 3: In-Focus Mask를 표시 (초점 영역은 흰색, 나머지는 검은색)
    // return float4(inFocusMask, inFocusMask, inFocusMask, 1.0f);
    
    // 예시 4: Mapped Signed CoC 값을 회색조로 표시
    // (0.5 근처는 중간 회색, 0은 검은색, 1은 흰색)
    //return float4(mappedSignedCoC, mappedSignedCoC, mappedSignedCoC, 1.0f);

    // 예시 5: 니어/파/인포커스를 다른 색으로 표시
    float3 debugColor = float3(0.0f, 0.0f, 0.0f);
    if (inFocusMask > 0.5f) // 초점 영역
    {
        //debugColor = float3(0.0f, 1.0f, 0.0f); // 녹색
    }
    else if (nearCoC > 0.01f) // 근경 아웃포커스
    {
        // nearCoC 값에 따라 밝기 조절 가능
        debugColor = float3(nearCoC, 0.0f, 0.0f); // 빨간색 (밝을수록 강한 근경 CoC)
    }
    else if (farCoC > 0.01f) // 원경 아웃포커스
    {
        // farCoC 값에 따라 밝기 조절 가능
        debugColor = float3(0.0f, 0.0f, farCoC); // 파란색 (밝을수록 강한 원경 CoC)
    }
    return float4(debugColor, 1.0f);

    // 예시 6: 원본 Signed CoC 값을 (어느 정도) 복원하여 표시 (MappedSignedCoC 사용)
    // DepthOfFieldConstant 버퍼에서 MaxAbsCocForAlphaEncoding 값을 가져와야 함
    // float maxAbsCoc = MaxAbsCocForAlphaEncoding; // 상수 버퍼에서 가져온 값이라고 가정
    // float originalScaledSignedCoC = (mappedSignedCoC * 2.0f - 1.0f) * maxAbsCoc;
    // // 시각화를 위해 0 중심으로 다시 매핑 (예: -maxAbsCoc -> 0, 0 -> 0.5, +maxAbsCoc -> 1)
    // float visualizedSignedCoC = saturate((originalScaledSignedCoC / maxAbsCoc) * 0.5f + 0.5f);
    // return float4(visualizedSignedCoC, visualizedSignedCoC, visualizedSignedCoC, 1.0f);

    // 예시 7: FarCoC는 R, NearCoC는 G, InFocusMask는 B에 표시
    // return float4(farCoC, nearCoC, inFocusMask, 1.0f);
}

float4 PS_Composite(PS_Input Input) : SV_TARGET
{
    // 1. 원본 해상도 레이어 정보 가져오기
    // LayerInfoTexture는 픽셀 단위의 정확한 마스크 정보이므로 PointSampler를 사용할 수 있으나,
    // 약간의 부드러운 전환을 위해 LinearSampler를 사용해도 무방합니다.
    // 여기서는 생성 시 의도에 따라 PointSampler 또는 LinearSampler를 선택합니다.
    // 일반적으로 CoC 값은 부드럽게 변하므로 LinearSampler가 더 적합할 수 있습니다.
    float4 LayerInfo = LayerInfoTexture.Sample(LinearSampler, Input.UV);
    float FarCoC = LayerInfo.r;
    float NearCoC = LayerInfo.g;
    float InFocusMask = LayerInfo.b; // 0.0 (아웃포커스) 또는 1.0 (인포커스)

    // 2. 원본 씬 컬러 가져오기
    float4 OriginalSceneColor = SceneTexture.Sample(LinearSampler, Input.UV);

    // 3. 블러된 파 레이어 컬러 가져오기 (업샘플링 발생)
    // BlurredFarLayerTexture의 알파 채널은 FarCoC를 담고 있을 수 있음 (블러 쉐이더 설계에 따라).
    // 여기서는 해당 CoC 값을 블러 강도에 이미 반영했다고 가정하고, RGB만 사용.
    // 또는, 해당 알파값을 추가적인 가중치로 사용할 수도 있습니다.
    float4 BlurredFarColor = BlurredFarTexture.Sample(LinearSampler, Input.UV);

    // 4. 블러된 니어 레이어 컬러 가져오기 (업샘플링 발생)
    float4 BlurredNearColor = BlurredNearTexture.Sample(LinearSampler, Input.UV);


    // 5. 합성 로직
    float3 FinalColor = OriginalSceneColor.rgb;

    // 파 레이어 합성:
    // inFocusMask가 0에 가까울수록 (아웃포커스), farCoC가 클수록 파 블러를 더 많이 반영.
    // (1.0 - inFocusMask)는 아웃포커스 정도를 나타냄.
    // farCoC는 해당 픽셀이 얼마나 파 필드에 속하며 흐려져야 하는지를 나타냄.
    // 두 값을 곱하여 최종 가중치로 사용할 수 있습니다.
    float FarBlendFactor = saturate((1.0f - InFocusMask) * FarCoC * 1.0);
    //FinalColor = lerp(FinalColor, BlurredFarColor.rgb, FarBlendFactor);
    FinalColor = lerp(FinalColor, BlurredFarColor.rgb, FarCoC);

    // 니어 레이어 합성:
    // 니어 블러는 파 블러 위에 덮어씌워지거나, 더 강하게 적용될 수 있음.
    // 여기서 주의: 니어 블러가 파 블러를 완전히 덮어쓰도록 할 것인가,
    // 아니면 파 블러와 혼합될 여지를 남길 것인가?
    // 일반적인 듀얼 레이어는 니어 레이어가 파 레이어 및 초점 영역보다 앞에 그려짐을 시뮬레이션.
    // 따라서, 니어 블러의 혼합은 이전 단계의 결과(finalColor)와 이루어짐.
    float NearBlendFactor = saturate((1.0f - InFocusMask) * NearCoC * 1.0);
    //FinalColor = lerp(FinalColor, BlurredNearColor.rgb, NearBlendFactor);
    FinalColor = lerp(FinalColor, BlurredNearColor.rgb, NearCoC);


    // 만약 니어 레이어가 존재할 때 파 레이어의 기여를 완전히 없애고 싶다면,
    // 또는 초점 영역과만 블렌딩하고 싶다면 로직 수정 필요.
    // 예시: 엄격한 레이어 우선순위
    /*
    if (InFocusMask > 0.5f) // 0.5 임계값
    {
        FinalColor = OriginalSceneColor.rgb;
    }
    else if (NearCoC > FarCoC && NearCoC > 0.01f) // 니어가 우세하고 유의미한 경우
    {
        // nearCoC 값을 블렌드 팩터로 직접 사용 (이미 0-1 범위)
        // 또는 blurredNearColor.a (만약 블러 쉐이더에서 CoC를 알파에 저장했다면)
        FinalColor = lerp(OriginalSceneColor.rgb, BlurredFarColor.rgb, NearCoC * 1.0);
    }
    else if (FarCoC > 0.01f) // 파가 우세하거나 니어가 없는 경우
    {
        FinalColor = lerp(OriginalSceneColor.rgb, BlurredFarColor.rgb, FarCoC * 1.0);
    }
    // else (모든 CoC가 매우 작으면) finalColor = originalSceneColor.rgb; (이미 초기값)
    */

    //return DebugCoC(Input.UV);

    // 최종 알파는 원본 씬의 알파를 사용
    return float4(FinalColor, OriginalSceneColor.a);
}

float4 CalculateDilate(float2 TexUV, float2 TextureSize, float DilateRadius)
{
    float4 MaxSample = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float MaxCoC = 0.0f;
    bool HasValidSample = false;

    // 확장 반경 내에서 최대 CoC와 해당 색상을 찾음
    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        float2 PixelOffset = PoissonSamples[i] * DilateRadius;
        float2 UVOffset = PixelOffset / TextureSize;
        float2 SampleUV = TexUV + UVOffset;

        float4 SampleData = SceneTexture.Sample(LinearSampler, SampleUV);
        float SampleCoC = SampleData.a;

        // Near 레이어에서만 작동 (음수 CoC 또는 특정 조건)
        // 여기서는 LayerInfoTexture의 Near CoC를 체크
        float4 LayerInfo = LayerInfoTexture.Sample(LinearSampler, SampleUV);
        float NearCoC = LayerInfo.g;

        if (NearCoC > MaxCoC)
        {
            MaxCoC = NearCoC;
            MaxSample = SampleData;
            HasValidSample = true;
        }
    }

    if (HasValidSample)
    {
        float3 originalColor = SceneTexture.Sample(LinearSampler, TexUV).rgb;
        //float3 dilatedColor = lerp(originalColor, MaxSample.rgb, saturate(MaxCoC * some_blend_factor));
        return float4(originalColor.rgb, MaxCoC);
    }
    // 유효한 샘플이 없으면 원본 반환
    return SceneTexture.Sample(LinearSampler, TexUV);
}

float4 CalculateStandardBlur(float2 TexUV, float2 TextureSize, float CoC)
{
    float4 CenterPixelData = SceneTexture.Sample(LinearSampler, TexUV);
    float3 AccumulatedColor = float3(0.0f, 0.0f, 0.0f);
    float TotalWeight = 0.0f;

    float BlurRadius = CoC * MAX_KERNEL_PIXEL_RADIUS * BokehIntensityScale;

    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        float2 PixelOffset = PoissonSamples[i] * BlurRadius;
        float2 UVOffset = PixelOffset / TextureSize;
        float2 SampleUV = TexUV + UVOffset;

        float4 SampleData = SceneTexture.Sample(LinearSampler, SampleUV);
        
        float Weight = 1.0f;
        
        // 가우시안 가중치
        float NormalizedDistance = length(PoissonSamples[i]);
        Weight *= exp(-NormalizedDistance * NormalizedDistance / (2.0f * 0.5f * 0.5f));

        // 하이라이트 부스트
        float HighlightBoost = 1.0f + saturate(dot(SampleData.rgb, SampleData.rgb) - 0.8f) * 3.0f;
        Weight *= HighlightBoost;

        AccumulatedColor += SampleData.rgb * Weight;
        TotalWeight += Weight;
    }

    if (TotalWeight > 0.001f)
    {
        AccumulatedColor /= TotalWeight;
    }
    else
    {
        AccumulatedColor = CenterPixelData.rgb;
    }

    return float4(AccumulatedColor, CoC);
}

float4 CalculateNearBlurDilateErode(float2 TexUV, float2 TextureSize)
{
    // 1단계: 현재 픽셀의 레이어 정보 확인
    float4 LayerInfo = LayerInfoTexture.Sample(LinearSampler, TexUV);
    float NearCoC = LayerInfo.g;
    float InFocusMask = LayerInfo.b;

    // Near 영역이 아니면 주변에서 Near 색상을 찾아서 가져옴
    if (NearCoC < 0.05f)
    {
        float3 AccumulatedColor = float3(0.0f, 0.0f, 0.0f);
        float TotalWeight = 0.0f;
        float4 OriginalColor = SceneTexture.Sample(LinearSampler, TexUV);

        // 확장된 반경에서 Near 픽셀들을 찾음
        float SearchRadius = MAX_KERNEL_PIXEL_RADIUS * BokehIntensityScale * 2.0f;

        for (int i = 0; i < NUM_SAMPLES; ++i)
        {
            float2 PixelOffset = PoissonSamples[i] * SearchRadius;
            float2 UVOffset = PixelOffset / TextureSize;
            float2 SampleUV = TexUV + UVOffset;

            float4 SampleLayerInfo = LayerInfoTexture.Sample(LinearSampler, SampleUV);
            float SampleNearCoC = SampleLayerInfo.g;

            if (SampleNearCoC > 0.1f) // 유의미한 Near CoC
            {
                float4 SampleColor = SceneTexture.Sample(LinearSampler, SampleUV);
                
                // 샘플 픽셀의 영향 반경 계산
                float SampleInfluenceRadius = SampleNearCoC * MAX_KERNEL_PIXEL_RADIUS * BokehIntensityScale;
                float DistanceToSample = length(PixelOffset);

                // 영향 반경 내에 있는지 확인
                if (DistanceToSample <= SampleInfluenceRadius)
                {
                    float Weight = saturate(1.0f - (DistanceToSample / max(0.001f, SampleInfluenceRadius)));
                    Weight *= SampleNearCoC; // CoC 강도 반영
                    Weight *= saturate(1.0f - InFocusMask); // 초점 영역에서는 약하게

                    // 하이라이트 부스트
                    float HighlightBoost = 1.0f + saturate(dot(SampleColor.rgb, SampleColor.rgb) - 0.8f) * 4.0f;
                    Weight *= HighlightBoost;

                    AccumulatedColor += SampleColor.rgb * Weight;
                    TotalWeight += Weight;
                }
            }
        }

        if (TotalWeight > 0.01f)
        {
            float BlendFactor = saturate(TotalWeight * 0.3f); // 블렌딩 강도 조절
            float3 BlurredColor = AccumulatedColor / TotalWeight;
            float3 FinalColor = lerp(OriginalColor.rgb, BlurredColor, BlendFactor);
            return float4(FinalColor, OriginalColor.a);
        }
        else
        {
            return OriginalColor;
        }
    }
    else
    {
        // Near 오브젝트 내부에서는 일반적인 블러 적용
        return CalculateStandardBlur(TexUV, TextureSize, NearCoC);
    }
}

float4 PS_DilateNear(PS_Input Input) : SV_TARGET
{
    float2 TextureSize = 1.0f / TexturePixelSize;
    float DilateRadius = MAX_KERNEL_PIXEL_RADIUS * BokehIntensityScale * 1.5f;
    return CalculateDilate(Input.UV, TextureSize, DilateRadius);
}

float4 PS_BlurNearLayerImproved(PS_Input Input) : SV_TARGET
{
    float2 TextureSize = 1.0f / TexturePixelSize;
    return CalculateNearBlurDilateErode(Input.UV, TextureSize);
}
