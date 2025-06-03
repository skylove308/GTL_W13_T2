#pragma once
#include "GameFramework/Actor.h"

struct FMap;

enum class EGameState
{
    None = 0,
    WaitingToStart,
    Playing,
    GameOver,
    Exit,
    Restart
};

class FMapModule;
class AGameManager : public AActor
{
    DECLARE_CLASS(AGameManager, AActor)

public:
    AGameManager();
    virtual ~AGameManager();

    EGameState GameState = EGameState::WaitingToStart;
    int Score = 0;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    EGameState GetState() const { return GameState; }
    void SetState(EGameState State);
    int GetScore() const { return Score; }
    void SetScore(int NewScore) { Score = NewScore; }
    void SpawnMap();
    void DestroyMap();

    void StartGame();
    void ExitGame();
    FMapModule* GetMapModule() const { return MapModule; }

private:
    FMapModule* MapModule = nullptr;
};
