#pragma once
#include "GameFramework/Actor.h"

class AGameUIManager : public AActor
{
    DECLARE_CLASS(AGameUIManager, AActor)

public:
    AGameUIManager();
    
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void RegisterLuaType(sol::state& Lua) override;
    virtual bool BindSelfLuaProperties() override;

private:
    int CurrentScore = 0;

public:
    // getter
    int GetScore() const { return CurrentScore; }

    // setter
    void SetScore(float NewScore) { CurrentScore = NewScore; }
};
