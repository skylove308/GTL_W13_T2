#include "GameManager.h"

#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "World/World.h"
#include "GameFramework/Character.h"

AGameManager::AGameManager()
{
}

AGameManager::~AGameManager()
{
}

void AGameManager::BeginPlay()
{
    AActor::BeginPlay();
    SetState(EGameState::WaitingToStart);
    Score = 0;
}

void AGameManager::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);
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
