#include "MapModule.h"
#include "Actors/Road.h"
#include "Engine/Engine.h"
#include "World/World.h"
#include "Math/MathUtility.h"

void FMapModule::Initialize()
{

}

void FMapModule::SpawnRoadMap()
{
    return;

    ARoad* Road = GEngine->ActiveWorld->SpawnActor<ARoad>();
    if (Road)
    {
        Road->Initialize(ERoadState::Safe, SpawnLocation);
    }

    SpawnLocation += FVector(130.0f, -50000.0f, -1.0f);

    int RandomCarRoadCount = FMath::RandHelper(4) + 1;

    for(int i= 0; i < RandomCarRoadCount; ++i)
    {
        ARoad* CarRoad = GEngine->ActiveWorld->SpawnActor<ARoad>();
        if (CarRoad)
        {
            CarRoad->Initialize(ERoadState::Car, SpawnLocation);
        }
        SpawnLocation += FVector(385.0f, 0.0f, 0.0f);
    }

    SpawnLocation += FVector(145.0f, 50000.0f, 1.0f);

}

