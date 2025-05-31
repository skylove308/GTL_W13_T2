#pragma once
#pragma once
#include "Vector.h"

namespace Shape
{
    struct FRay
    {
        FVector Origin;
        FVector Direction;
    };

    struct FSphere
    {
        FVector Center;
        float Radius;
    };

    struct FBox
    {
        FVector Min;
        FVector Max;
    };

    struct FOrientedBox
    {
        FVector AxisX = FVector(1, 0, 0);
        FVector AxisY = FVector(0, 1, 0);
        FVector AxisZ = FVector(0, 0, 1);
        FVector Center = FVector(0, 0, 0);
        float ExtentX = 1.0f;
        float ExtentY = 1.0f;
        float ExtentZ = 1.0f;
    };

    struct FCapsule
    {
        FVector A; // 캡슐의 한쪽 끝
        FVector B; // 캡슐의 다른쪽 끝
        float Radius;
    };

    // Xx + Yy + Zz + W = 0;
    struct FPlane
    {
        float X, Y, Z, W;
    };

    struct FCone
    {
        FVector ApexPosition; // 원뿔의 꼭지점 위치
        float Radius;         // 원뿔의 밑면 반지름
        FVector Direction;    // 원뿔의 방향 벡터
        float Angle;          // 원뿔의 각도 (라디안 단위)
    };

    struct FEllipticalCone
    {
        FVector ApexPosition; // 원뿔의 꼭지점 위치
        FVector Direction;    // 원뿔의 방향 벡터
        float AngleWidth;
        float AngleHeight;
        float Radius;
    };
};
