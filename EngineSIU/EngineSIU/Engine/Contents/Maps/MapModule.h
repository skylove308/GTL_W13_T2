#pragma once
#include "Math/Vector.h"
#include "Container/Array.h"
#include <queue>

class ARoad;
struct FMap
{
    TArray<ARoad*> Roads;
};

class FMapModule
{
public:
    FMapModule() = default;
    ~FMapModule() = default;

    void Initialize();
    void SpawnRoadMap();
    void DestroyRoadMap();

    FVector SpawnLocation = FVector(0.0f, 0.0f, 1.0f);
    int MapSize = 0;

private:
    std::queue<FMap*> Maps;
};

