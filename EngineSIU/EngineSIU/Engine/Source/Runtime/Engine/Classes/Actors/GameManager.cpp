#include "GameManager.h"

#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "World/World.h"
#include "GameFramework/Character.h"
#include "Engine/Contents/Maps/MapModule.h"

AGameManager::AGameManager()
{
    MapModule = new FMapModule();
}

AGameManager::~AGameManager()
{
}

void AGameManager::BeginPlay()
{
    Super::BeginPlay();

    SetState(EGameState::WaitingToStart);
    Score = 0;
}

void AGameManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AGameManager::SetState(EGameState State)
{
    GameState = State;
    switch (State)
    {
    case EGameState::Restart:
    {
        ExitGame();
        StartGame();
        break;
    }
    case EGameState::Exit:
    {
        ExitGame();
        break;
    }
    default:
        break;
    }
}

void AGameManager::SpawnMap(int MaxRoadNum)
{
    MapModule->SpawnRoadMap(MaxRoadNum);
}

void AGameManager::DestroyMap()
{
    if (MapModule->MapSize > 12)
    {
        MapModule->DestroyRoadMap();
    }
}

void AGameManager::StartGame()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    Engine->StartPIE();
}

void AGameManager::ExitGame()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    Engine->EndPIE();
}
