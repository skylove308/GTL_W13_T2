#include "MapModule.h"
#include "Actors/Road.h"
#include "Engine/Engine.h"
#include "World/World.h"

void FMapModule::Initialize() 
{

}

void FMapModule::SpawnRoad()
{
    ARoad* Road = GEngine->ActiveWorld->SpawnActor<ARoad>();
    if (Road)
    {
        Road->Initialize(ERoadState::Safe, SpawnLocation);
        Road->Initialize(ERoadState::Safe, SpawnLocation);
    }
}
