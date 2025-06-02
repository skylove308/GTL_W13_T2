#include "GameManager.h"

AGameManager::AGameManager()
{
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
    switch (State)
    {
    case EGameState::WaitingToStart:
    {
        break;
    }
    case EGameState::Playing:
    {
        // Playing 상태일 때 계속해서 검사할 로직 (타이머, 적 스폰 등)
        break;
    }
    case EGameState::GameOver:
    {
        // GameOver 상태에서 재시작 대기 로직 등
        break;
    }
    default:
        break;
    }
}

void AGameManager::StartGame()
{
}

void AGameManager::EndGame()
{
}

void AGameManager::RestartGame()
{
}
