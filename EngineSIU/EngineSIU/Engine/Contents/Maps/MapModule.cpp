#include "MapModule.h"
#include "Actors/Road.h"
#include "Actors/StreetLight.h"
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
        Road->Initialize(ERoadState::Safe, Map, SpawnLocation);
        Map->Roads.Add(Road);
    }

    if (Road->GetCurrentRoadState() == ERoadState::Safe)
    {
        int StreetLightCount = 3;
        float Spacing = 1000.0f;

        float randomShift = FMath::FRandRange(-Spacing * 0.5f, Spacing * 0.5f);

        int mid = StreetLightCount / 2;

        for (int i = 0; i < StreetLightCount; ++i)
        {
            AStreetLight* Light = GEngine->ActiveWorld->SpawnActor<AStreetLight>();
            if (Light)
            {
                float yOffset = ( (float)i - (float)mid ) * Spacing + randomShift;

                FVector LightLocation = SpawnLocation + FVector(120.0f, yOffset + 200.0f, 300.0f);
                Light->Initialize(LightLocation);
                Map->StreetLights.Add(Light);
            }
        }
    }
    
    SpawnLocation += FVector(600.0f, -0.0f, 0.0f);

    int RandomCarRoadCount = 1;// FMath::RandHelper(4) + MaxRoadNum;
    
    for(int i= 0; i < RandomCarRoadCount; ++i)
    {
        ARoad* CarRoad = GEngine->ActiveWorld->SpawnActor<ARoad>();
        if (CarRoad)
        {
            CarRoad->Initialize(ERoadState::Car, Map, SpawnLocation);
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

            for (AStreetLight* Light : Map->StreetLights)
            {
                if (Light)
                {
                    Light->Destroy();
                }
            }
        }
        MapSize--;
        delete Map;
    }
}
