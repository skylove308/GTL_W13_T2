#pragma once
#include "Gameframework/Actor.h"

DECLARE_MULTICAST_DELEGATE(FOnCharacterDeath);

enum class ERoadState : uint8
{
    Safe,
    Warning,
    Danger,
    Car,
    Destroy
};

class UStaticMeshComponent;
class ARoad : public AActor
{
    DECLARE_CLASS(ARoad, AActor)

public:
    ARoad();

    void Initialize(ERoadState RoadState, FVector SpawnWorldLocation = FVector(0.0f, 0.0f, 0.0f));
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    
    void OnOverlappedRoad(float DeltaTime);
    void DestroyRoad();

    virtual void RegisterLuaType(sol::state& Lua) override;
    virtual bool BindSelfLuaProperties() override;

    void SetCurrentRoadState(ERoadState State) { CurrentRoadState = State; }
    ERoadState GetCurrentRoadState() const { return CurrentRoadState; }

    void SetCurrentRoadTime(int32 Time) { CurrentRoadTime = Time; }
    int32 GetCurrentRoadTime() const { return CurrentRoadTime; }

    void SetSafeJoneTime(int32 Time) { SafeJoneTime = Time; }
    int32 GetSafeJoneTime() const { return SafeJoneTime; }

    void SetWarningJoneTime(int32 Time) { WarningJoneTime = Time; }
    int32 GetWarningJoneTime() const { return WarningJoneTime; }

    void SetIsOverlapped(bool bOverlapped) { bIsOverlapped = bOverlapped; }

public:
    FOnCharacterDeath OnDeath;

private:
    UStaticMeshComponent* RoadMesh = nullptr;
    ERoadState CurrentRoadState = ERoadState::Safe;
    int32 CurrentRoadTime = 0.0f;
    int32 SafeJoneTime = 5.0f;
    int32 WarningJoneTime = 5.0f;
    bool bIsOverlapped = false;
};

