#include "ShaderRegisters.hlsl"

#define NUM_CONSTANTS 512
struct VS_INPUT_POS_ONLY
{
    float3 position : POSITION0;
};

struct PS_INPUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

struct Sphere
{
    float3 Center;
    float Radius;
    float4 Color;
};

float4 DefaultPS(PS_INPUT input) : SV_Target
{
    return input.color;
}

cbuffer SphereConstants : register(b11)
{
    Sphere Spheres[NUM_CONSTANTS];
}

PS_INPUT SphereVS(VS_INPUT_POS_ONLY input, uint instanceID : SV_InstanceID)
{
    PS_INPUT output;
    
    float3 pos = Spheres[instanceID].Center;
    float scale = Spheres[instanceID].Radius;
    
    float4 localPos = float4(input.position.xyz * scale + pos, 1.f);
        
    localPos = mul(localPos, ViewMatrix);
    localPos = mul(localPos, ProjectionMatrix);
    output.position = localPos;

    output.color = Spheres[instanceID].Color;
    
    return output;
}

struct Capsule
{
    float3 A; // 캡슐의 한쪽 끝
    float Radius;
    float3 B; // 캡슐의 다른쪽 끝
    float Pad0;
    float4 Color;
};

cbuffer SphereConstants : register(b11)
{
    Capsule Capsules[NUM_CONSTANTS];
}

PS_INPUT CapsuleVS(VS_INPUT_POS_ONLY input, uint VertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    PS_INPUT output;

    // --- 캡슐 파라미터 추출 ---
    float3 CapsuleA = Capsules[instanceID].A;
    float3 CapsuleB = Capsules[instanceID].B;
    float CapsuleRadius = Capsules[instanceID].Radius;
    float4 CapsuleColor = Capsules[instanceID].Color;
    float3 CapsuleDir = normalize(CapsuleB - CapsuleA);
    float CapsuleLen = length(CapsuleB - CapsuleA);
    float CapsuleHalfHeight = CapsuleLen * 0.5f;
    float3 center = (CapsuleA + CapsuleB) * 0.5;

    // --- 메시 생성 규칙 ---
    const int NumRings = 32;
    const int NumSegments = 32;
    float halfRings = NumRings * 0.5f;
    float vertsPerRing = NumSegments + 1;
    float topHemiVertCount = (halfRings + 1) * vertsPerRing;
    float cylinderVertCount = 2 * vertsPerRing;
    float bottomHemiVertCount = (halfRings + 1) * vertsPerRing;

    uint cylinderStart = (uint) topHemiVertCount;
    uint cylinderEnd = cylinderStart + (uint) cylinderVertCount;

    // --- Step 1: 메시를 먼저 +y축이 CapsuleDir이 되도록 회전 ---
    // input.position은 캡슐 로컬 기준(세로로 선 상태, y축이 중심축)
    float3 meshPos = input.position;

    // 로드리게스 공식으로 y축(0,1,0)->CapsuleDir로 회전
    float3 up = float3(0, 1, 0);
    float3 axis = cross(up, CapsuleDir);
    float sinTheta = length(axis);
    float cosTheta = dot(up, CapsuleDir);

    float3 rotatedPos;
    if (sinTheta < 1e-5)
    {
        // 이미 같은 방향이거나 반대 방향
        rotatedPos = (cosTheta > 0) ? meshPos : float3(meshPos.x, -meshPos.y, -meshPos.z);
    }
    else
    {
        axis = normalize(axis);
        float3 v = axis;
        float3 p = meshPos;
        // 로드리게스 공식
        rotatedPos = p * cosTheta + cross(v, p) * sinTheta + v * dot(v, p) * (1 - cosTheta);
    }

    // --- Step 2: Cylinder 구간만 CapsuleDir 방향(이미 회전된 y축)으로 ±CapsuleHalfHeight 오프셋 ---
    float3 offset = float3(0, 0, 0);
    if (VertexID < cylinderStart)
    {
        offset = CapsuleDir * CapsuleHalfHeight;
    }
    else if (VertexID >= cylinderEnd)
    {
        offset = -CapsuleDir * CapsuleHalfHeight;
    }
    else
    {
        offset = -CapsuleDir * CapsuleHalfHeight;
    }

    // --- Step 3: 반지름 적용 ---
    rotatedPos *= CapsuleRadius;

    // --- Step 4: 최종 위치 계산 ---
    float3 worldPos = center + rotatedPos + offset;

    // --- Step 5: 변환 ---
    float4 localPos = float4(worldPos, 1.0f);
    localPos = mul(localPos, ViewMatrix);
    localPos = mul(localPos, ProjectionMatrix);

    output.position = localPos;
    output.color = CapsuleColor;
    return output;
}


struct OrientedBox
{
    float3 AxisX;
    float ExtentX;
    float3 AxisY;
    float ExtentY;
    float3 AxisZ;
    float ExtentZ;
    float3 Center;
    float Pad0;
    float4 Color;
};

cbuffer OrientedBoxConstants : register(b11)
{
    OrientedBox OrientedBoxes[NUM_CONSTANTS];
}

//// 8개 박스 버텍스의 로컬 좌표를 하드코딩 (vertexID 기반)
//float3 GetBoxVertexPosition(uint vertexID)
//{
//    // vertexID: 0~7
//    // -1 또는 +1로 각 축의 꼭짓점을 할당
//    // vertex 순서 예시: (Unreal Engine DrawDebugBox 참고)
//    // 0: (-1, -1, -1)
//    // 1: (+1, -1, -1)
//    // 2: (+1, +1, -1)
//    // 3: (-1, +1, -1)
//    // 4: (-1, -1, +1)
//    // 5: (+1, -1, +1)
//    // 6: (+1, +1, +1)
//    // 7: (-1, +1, +1)
//    float3 verts[8] =
//    {
//        float3(-1, -1, -1),
//        float3(+1, -1, -1),
//        float3(+1, +1, -1),
//        float3(-1, +1, -1),
//        float3(-1, -1, +1),
//        float3(+1, -1, +1),
//        float3(+1, +1, +1),
//        float3(-1, +1, +1)
//    };
//    return verts[vertexID % 8];
//}


float3 GetOrientedBoxVertex(uint vertexID, OrientedBox box)
{
    // vertexID: 0~7
    float signX = ((vertexID >> 0) & 1) ? 1.0f : -1.0f;
    float signY = ((vertexID >> 1) & 1) ? 1.0f : -1.0f;
    float signZ = ((vertexID >> 2) & 1) ? 1.0f : -1.0f;

    return box.Center +
           signX * box.AxisX * box.ExtentX +
           signY * box.AxisY * box.ExtentY +
           signZ * box.AxisZ * box.ExtentZ;
}

PS_INPUT OrientedBoxVS(VS_INPUT_POS_ONLY input, uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    PS_INPUT output;

    OrientedBox box = OrientedBoxes[instanceID];

    float3 worldPos = GetOrientedBoxVertex(vertexID % 8, box);

    float4 clipPos = mul(float4(worldPos, 1.0f), ViewMatrix);
    clipPos = mul(clipPos, ProjectionMatrix);

    output.position = clipPos;
    output.color = box.Color;
    return output;
}

struct Cone
{
    float3 Origin;
    float Pad0;
    float3 Direction;
    float Pad1;
    float Length;
    float AngleWidth;
    float AngleHeight;
    float Pad2;
    float4 Color;
};

cbuffer ConeConstants : register(b11)
{
    Cone Cones[NUM_CONSTANTS];
}

PS_INPUT ConeVS(VS_INPUT_POS_ONLY input, uint VertexID : SV_VertexID, uint InstanceID : SV_InstanceID)
{
    PS_INPUT output;
    Cone cone = Cones[InstanceID];

    const uint NumSides = 32;
    uint vID = VertexID;

    float3 worldPos;

    if (vID == 0)
    {
        // 0번 버텍스: 원뿔의 꼭짓점 (Origin)
        worldPos = cone.Origin;
    }
    else
    {
        // 1~NumSides: 원주 버텍스
        uint i = vID - 1;
        float Angle1 = clamp(cone.AngleHeight, 1e-5, 3.141592f - 1e-5);
        float Angle2 = clamp(cone.AngleWidth, 1e-5, 3.141592f - 1e-5);

        float SinX_2 = sin(0.5f * Angle1);
        float SinY_2 = sin(0.5f * Angle2);

        float SinSqX_2 = SinX_2 * SinX_2;
        float SinSqY_2 = SinY_2 * SinY_2;

        float Fraction = (float) i / NumSides;
        float Thi = 2.0f * 3.141592f * Fraction;
        float Phi = atan2(sin(Thi) * SinY_2, cos(Thi) * SinX_2);
        float SinPhi = sin(Phi);
        float CosPhi = cos(Phi);
        float SinSqPhi = SinPhi * SinPhi;
        float CosSqPhi = CosPhi * CosPhi;

        float RSq = SinSqX_2 * SinSqY_2 / (SinSqX_2 * SinSqPhi + SinSqY_2 * CosSqPhi);
        float R = sqrt(RSq);
        float Sqr = sqrt(1.0f - RSq);
        float Alpha = R * CosPhi;
        float Beta = R * SinPhi;

        float3 coneVert;
        coneVert.x = (1.0f - 2.0f * RSq);
        coneVert.y = 2.0f * Sqr * Alpha;
        coneVert.z = 2.0f * Sqr * Beta;

        float3 Dir = normalize(cone.Direction);
        float3 YAxis, ZAxis;
        float3 Up = abs(Dir.z) < 0.999f ? float3(0, 0, 1) : float3(1, 0, 0);
        YAxis = normalize(cross(Up, Dir));
        ZAxis = cross(Dir, YAxis);

        worldPos = cone.Origin +
                    coneVert.x * Dir * cone.Length +
                    coneVert.y * YAxis * cone.Length +
                    coneVert.z * ZAxis * cone.Length;
    }

    float4 clipPos = mul(float4(worldPos, 1.0f), ViewMatrix);
    clipPos = mul(clipPos, ProjectionMatrix);

    output.position = clipPos;
    output.color = cone.Color;
    
    return output;
}
