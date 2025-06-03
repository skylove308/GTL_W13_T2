#pragma once
#include "Math/Vector.h"
#include "Container/Array.h"
#include <queue>

class AStreetLight;
class ARoad;

struct FMap
{
    TArray<ARoad*> Roads;
    TArray<AStreetLight*> StreetLights;
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

