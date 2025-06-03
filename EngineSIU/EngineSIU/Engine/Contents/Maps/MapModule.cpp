#include "MapModule.h"
#include "Actors/Road.h"
#include "Engine/Engine.h"
#include "World/World.h"
#include "Math/MathUtility.h"

void FMapModule::Initialize()
{

}

void FMapModule::SpawnRoadMap(int MaxRoadNum)
{
    FMap* Map = new FMap();

    ARoad* Road = GEngine->ActiveWorld->SpawnActor<ARoad>();
    if (Road)
    {
        Road->Initialize(ERoadState::Safe, SpawnLocation);
        Map->Roads.Add(Road);
    }

    SpawnLocation += FVector(600.0f, -0.0f, 0.0f);

    int RandomCarRoadCount = FMath::RandHelper(4) + MaxRoadNum;

    for(int i= 0; i < RandomCarRoadCount; ++i)
    {
        ARoad* CarRoad = GEngine->ActiveWorld->SpawnActor<ARoad>();
        if (CarRoad)
        {
            CarRoad->Initialize(ERoadState::Car, SpawnLocation);
            Map->Roads.Add(CarRoad);
        }
        SpawnLocation += FVector(600.0f, 0.0f, 0.0f);
    }

    Maps.push(Map);
    MapSize++;
}

void FMapModule::DestroyRoadMap()
{
    if (Maps.size() > 0)
    {
        FMap* Map = Maps.front();
        Maps.pop();
        if (Map)
        {
            for(ARoad* Road : Map->Roads)
            {
                if (Road)
                {
                    Road->Destroy();
                }
            }
        }
        MapSize--;
        delete Map;
    }
}

