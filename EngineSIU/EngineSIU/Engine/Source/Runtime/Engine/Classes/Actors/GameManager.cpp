#include "GameManager.h"

#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "World/World.h"

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
    UE_LOG(ELogLevel::Error, TEXT("BeginPlay"));
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
    case EGameState::WaitingToStart:
    {
        UE_LOG(ELogLevel::Error, TEXT("WaitingToStart"));
        break;
    }
    case EGameState::Playing:
    {
        break;
    }
    case EGameState::GameOver:
    {
        
        break;
    }
    case EGameState::Exit:
    {
        UE_LOG(ELogLevel::Error, TEXT("Exit"));
        ExitGame();
        break;
    }
    default:
        break;
    }
}

void AGameManager::ExitGame()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    Engine->EndPIE();
}
