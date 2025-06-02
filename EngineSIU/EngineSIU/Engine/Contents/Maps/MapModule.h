#pragma once
#include "Math/Vector.h"

class FMapModule
{
public:
    FMapModule() = default;
    ~FMapModule() = default;

    void Initialize();
    void SpawnRoad();

    FVector SpawnLocation = FVector(0.0f, 0.0f, 0.0f);
};

