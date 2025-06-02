#pragma once
#include "GameFramework/Actor.h"

enum class EGameState
{
    WaitingToStart,
    Playing,
    GameOver
};

class AGameManager : public AActor
{
    DECLARE_CLASS(AGameManager, AActor)

public:
    static AGameManager& Instance() {
        static AGameManager instance;
        return instance;
    }
    
private:
    AGameManager();
    virtual ~AGameManager();

    EGameState GameState = EGameState::WaitingToStart;
    int Score = 0;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    EGameState GetState() const { return GameState; }
    void SetState(EGameState State);
    
    void StartGame();
    void EndGame();
    void RestartGame();
};
