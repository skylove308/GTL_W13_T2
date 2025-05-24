
#include "ShaderRegisters.hlsl"

Texture2D LayerInfoTexture : register(t90);
Texture2D BlurredNearTexture : register(t91);
Texture2D BlurredFarTexture : register(t92);
Texture2D InputDepth : register(t99);
Texture2D SceneTexture : register(t100);

SamplerState Sampler : register(s10);

static const float MAX_KERNEL_PIXEL_RADIUS = 8.0f;

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

float4 CalculateBokeh(float2 TexUV, float2 CurrentTextureSize)
{
    // 현재 픽셀의 정보 (색상 및 CoC)
    float4 CenterPixelData = SceneTexture.Sample(Sampler, TexUV);
    float3 CenterColor = CenterPixelData.rgb;
    float CoC = CenterPixelData.a; // NearCoC (0.0 ~ 1.0)

    // CoC가 매우 작거나, 해당 픽셀이 니어 레이어에 속하지 않으면 (알파가 0에 가까우면)
    // 원본 픽셀 색상 반환 (블러 없음)
    // NearCoC 값은 이미 0~1 범위로 가정. 추출 단계에서 0인 영역은 색상도 0,0,0 일 것.
    if (CoC < 0.01f) // 임계값은 조절 가능
    {
        // 알파값이 0인 픽셀은 보통 (0,0,0,0)이므로, 그대로 반환하면 됩니다.
        // 만약 알파가 0이지만 색상이 남아있는 경우가 있다면, 명시적으로 (0,0,0,0) 처리.
        return CenterPixelData; // 또는 float4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    float3 AccumulatedColor = float3(0.0f, 0.0f, 0.0f);
    float TotalWeight = 0.0f;

    // 실제 블러 반경 (픽셀 단위). NearCoC 값과 MAX_KERNEL_PIXEL_RADIUS를 곱하여 결정.
    // BokehIntensityScale을 추가로 곱해 전반적인 보케 크기 조절 가능.
    float ActualPixelRadius = CoC * MAX_KERNEL_PIXEL_RADIUS /* * BokehIntensityScale */;

    // 가우시안 시그마 값 계산 (기존 코드와 유사하게, 또는 다른 방식으로)
    // 실제 픽셀 반경에 비례하도록 설정. 너무 크면 과도하게 부드러워지고 작으면 샘플링 패턴이 보일 수 있음.
    float Sigma = ActualPixelRadius * 1.0f; // 이 값은 실험을 통해 조절
    // 분모 0 방지
    float TwoSigmaSquared = max(0.0001f, 2.0f * Sigma * Sigma);


    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        // 포아송 샘플 오프셋에 실제 픽셀 반경을 곱함
        float2 PixelOffset = PoissonSamples[i] * ActualPixelRadius;
        float2 UVOffset = PixelOffset / CurrentTextureSize; // 픽셀 오프셋을 UV 오프셋으로 변환

        float2 SampleUV = TexUV + UVOffset;

        // 샘플링할 픽셀의 데이터 (색상 및 해당 픽셀의 CoC)
        // 중요: 주변 픽셀도 니어 레이어에 속하고 유효한 CoC 값을 가져야 블러에 기여
        float4 SampleData = SceneTexture.Sample(Sampler, SampleUV);
        float3 SampleColor = SampleData.rgb;
        float SampleCoC = SampleData.a; // 샘플링된 픽셀의 CoC 값

        // 샘플링된 픽셀이 유효한 니어 픽셀인지 확인 (CoC 값 기반)
        // 또는, 추출 단계에서 이미 배경이 (0,0,0,0)으로 처리되었다면,
        // 색상이 검은색인 샘플은 기여도를 낮추거나 무시할 수 있습니다.
        if (SampleCoC > 0.01f) // 또는 sampleData.a > centerPixelData.a * 0.5f 등으로 하여 너무 다른 CoC는 덜 섞이게
        {
            // 거리 기반 가우시안 가중치
            // PoissonSamples[i]는 -1~1 범위의 정규화된 오프셋이므로, 이것으로 거리를 계산
            float DistanceSquared = dot(PoissonSamples[i], PoissonSamples[i]); // 중심에서의 정규화된 거리 제곱
            float Weight = exp(-DistanceSquared / TwoSigmaSquared); // 기본 가우시안 (시그마 0.5 가정)
                                                                     // 또는 twoSigmaSquared 사용: exp(-(length(pixelOffset) * length(pixelOffset)) / twoSigmaSquared);
            
            // (선택적) 보케 하이라이트 강조: 밝은 픽셀에 더 높은 가중치
            // float highlightBoost = 1.0f + saturate(dot(sampleColor, sampleColor) - 0.8f) * 2.0f;
            // weight *= highlightBoost;

            // (선택적) CoC 값에 따른 가중치: 현재 픽셀의 CoC와 샘플 픽셀의 CoC를 비교하여
            // 너무 차이가 크면 가중치를 줄이는 방식도 가능 (leak 방지)
            // float cocDifferenceFactor = 1.0f - saturate(abs(nearCoC - sampleCoC) / max(nearCoC, 0.01f));
            // weight *= cocDifferenceFactor;

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
        // 샘플을 하나도 못 찾았거나 가중치가 매우 낮으면 원본 색상 반환 (또는 (0,0,0,0))
        // 이 경우는 거의 발생하지 않아야 함 (최소한 중앙 샘플은 유효해야 함)
        // 만약 centerPixelData.a가 0에 가까워 위에서 이미 반환되었다면 이 코드는 실행되지 않음.
        return CenterPixelData;
    }

    // 최종 결과의 알파 값은 어떻게 할 것인가?
    // 1. 원본 CoC 값(nearCoC)을 그대로 유지: 합성 시 사용
    // 2. 1.0으로 설정: 블러 처리된 영역은 불투명하다고 가정
    // 여기서는 원본 CoC 값을 유지하여 합성 단계에서 활용하도록 합니다.
    return float4(AccumulatedColor, CoC);
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
    
    const float NonLinearDepth = InputDepth.Sample(Sampler, UV).r;
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

    //LayerInfo.w = ScaledSignedCoc; // Raw Scaled Signed CoC 저장

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

float4 PS_BlurLayer(PS_Input Input) : SV_TARGET
{
    float2 DownSampleTextureSize = TexturePixelSize;
    return CalculateBokeh(Input.UV, DownSampleTextureSize);
}

float4 PS_ExtractAndDownsampleLayer(PS_Input Input) : SV_TARGET
{
    float2 UV = Input.UV;
    float2 TexelSize = TexturePixelSize; // (1/width, 1/height) of full res texture

    // 2x2 샘플링 오프셋 (풀 해상도 텍셀 기준)
    // 다운샘플링된 픽셀의 중앙이 풀 해상도 픽셀 그리드의 교차점에 오도록 오프셋 조정 가능
    // 예: 출력 픽셀이 입력 픽셀 (i,j), (i+1,j), (i,j+1), (i+1,j+1)을 커버
    // 샘플링 위치는 각 입력 픽셀의 중앙이 됨
    float2 offsets[4] =
    {
        float2(-0.5f * TexelSize.x, -0.5f * TexelSize.y),
        float2( 0.5f * TexelSize.x, -0.5f * TexelSize.y),
        float2(-0.5f * TexelSize.x,  0.5f * TexelSize.y),
        float2( 0.5f * TexelSize.x,  0.5f * TexelSize.y)
    };

    float3 AccumulatedColor = float3(0.0f, 0.0f, 0.0f);
    float TotalWeight = 0.0f;
    float MaxCoC = 0.0f; // 합성 시 사용할 CoC 대표값 (예: 최대값)

    for (int i = 0; i < 4; ++i)
    {
        float2 SampleUV = UV + offsets[i];
        float4 CurrentLayerInfo = LayerInfoTexture.Sample(Sampler, SampleUV);
#ifdef NEAR
        float CurrentCoC = CurrentLayerInfo.g;
#else
        float CurrentCoC = CurrentLayerInfo.r;
#endif

        if (CurrentCoC > 0.0f)
        {
            float4 CurrentSceneColor = SceneTexture.Sample(Sampler, SampleUV);
            AccumulatedColor += CurrentSceneColor.rgb * CurrentCoC; // CoC를 가중치로 사용
            TotalWeight += CurrentCoC;
            MaxCoC = max(MaxCoC, CurrentCoC);
        }
    }

    if (TotalWeight > 0.001f)
    {
        return float4(AccumulatedColor / TotalWeight, MaxCoC); // 또는 totalNearWeight / 4.0f 등
    }
    
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 DebugCoC(float2 UV)
{
    float4 layerInfo = LayerInfoTexture.Sample(Sampler, UV); // 또는 LinearClampSampler
    
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
    // return float4(mappedSignedCoC, mappedSignedCoC, mappedSignedCoC, 1.0f);

    // 예시 5: 니어/파/인포커스를 다른 색으로 표시
    float3 debugColor = float3(0.0f, 0.0f, 0.0f);
    if (inFocusMask > 0.5f) // 초점 영역
    {
        debugColor = float3(0.0f, 1.0f, 0.0f); // 녹색
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
    float4 LayerInfo = LayerInfoTexture.Sample(Sampler, Input.UV);
    float FarCoC = LayerInfo.r;
    float NearCoC = LayerInfo.g;
    float InFocusMask = LayerInfo.b; // 0.0 (아웃포커스) 또는 1.0 (인포커스)

    // 2. 원본 씬 컬러 가져오기
    float4 OriginalSceneColor = SceneTexture.Sample(Sampler, Input.UV);

    // 3. 블러된 파 레이어 컬러 가져오기 (업샘플링 발생)
    // BlurredFarLayerTexture의 알파 채널은 FarCoC를 담고 있을 수 있음 (블러 쉐이더 설계에 따라).
    // 여기서는 해당 CoC 값을 블러 강도에 이미 반영했다고 가정하고, RGB만 사용.
    // 또는, 해당 알파값을 추가적인 가중치로 사용할 수도 있습니다.
    float4 BlurredFarColor = BlurredFarTexture.Sample(Sampler, Input.UV);

    // 4. 블러된 니어 레이어 컬러 가져오기 (업샘플링 발생)
    float4 BlurredNearColor = BlurredNearTexture.Sample(Sampler, Input.UV);


    // 5. 합성 로직
    float3 FinalColor = OriginalSceneColor.rgb;

    // 파 레이어 합성:
    // inFocusMask가 0에 가까울수록 (아웃포커스), farCoC가 클수록 파 블러를 더 많이 반영.
    // (1.0 - inFocusMask)는 아웃포커스 정도를 나타냄.
    // farCoC는 해당 픽셀이 얼마나 파 필드에 속하며 흐려져야 하는지를 나타냄.
    // 두 값을 곱하여 최종 가중치로 사용할 수 있습니다.
    float FarBlendFactor = saturate((1.0f - InFocusMask) * FarCoC * 1.0);
    FinalColor = lerp(FinalColor, BlurredFarColor.rgb, FarBlendFactor);

    // 니어 레이어 합성:
    // 니어 블러는 파 블러 위에 덮어씌워지거나, 더 강하게 적용될 수 있음.
    // 여기서 주의: 니어 블러가 파 블러를 완전히 덮어쓰도록 할 것인가,
    // 아니면 파 블러와 혼합될 여지를 남길 것인가?
    // 일반적인 듀얼 레이어는 니어 레이어가 파 레이어 및 초점 영역보다 앞에 그려짐을 시뮬레이션.
    // 따라서, 니어 블러의 혼합은 이전 단계의 결과(finalColor)와 이루어짐.
    float NearBlendFactor = saturate((1.0f - InFocusMask) * NearCoC * 1.0);
    FinalColor = lerp(FinalColor, BlurredNearColor.rgb, NearBlendFactor);


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
