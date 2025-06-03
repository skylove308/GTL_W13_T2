#pragma once
#include "Gameframework/Actor.h"

struct FMap;
DECLARE_MULTICAST_DELEGATE(FOnCharacterDeath);
DECLARE_MULTICAST_DELEGATE(FOnCharacterRed);
DECLARE_MULTICAST_DELEGATE(FOnCharacterNoRed);

enum class ERoadState : uint8
{
    Safe,
    Warning,
    Danger,
    Car,
    Destroy
};

class AGameManager;
class UStaticMeshComponent;
class ACar;
class ARoad : public AActor
{
    DECLARE_CLASS(ARoad, AActor)

public:
    ARoad();

    void Initialize(ERoadState RoadState, FMap* Map = nullptr, FVector SpawnWorldLocation = FVector(0.0f, 0.0f, 0.0f));
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    
    void OnOverlappedRoad(float DeltaTime);
    void DestroyRoad();

    virtual void RegisterLuaType(sol::state& Lua) override;
    virtual bool BindSelfLuaProperties() override;

    void SetCurrentRoadState(ERoadState State) { CurrentRoadState = State; }
    ERoadState GetCurrentRoadState() const { return CurrentRoadState; }

    void SetCurrentRoadTime(float Time) { CurrentRoadTime = Time; }
    float GetCurrentRoadTime() const { return CurrentRoadTime; }

    void SetSafeJoneTime(float Time) { SafeJoneTime = Time; }
    float GetSafeJoneTime() const { return SafeJoneTime; }

    void SetWarningJoneTime(float Time) { WarningJoneTime = Time; }
    float GetWarningJoneTime() const { return WarningJoneTime; }

    void SetIsOverlapped(bool bOverlapped) { bIsOverlapped = bOverlapped; }

    void TurnOnStreetLights();

public:
    FOnCharacterDeath OnDeath;
    FOnCharacterRed OnRed;
    FOnCharacterNoRed OnNoRed;

private:
    UStaticMeshComponent* RoadMesh = nullptr;
    ERoadState CurrentRoadState = ERoadState::Safe;
    AGameManager* GameManager = nullptr;
    float CurrentRoadTime = 0.0f;
    float SafeJoneTime = 2.0f;
    float WarningJoneTime = 2.0f;
    bool bIsOverlapped = false;
    bool bIsCarOnRoad = false;
    bool bIsFirstTimeOnRoad = false;
    ACar* CurrentCar = nullptr;
    float CarOnRoadTime = 0.0f;
    bool bIsRoadRightSpawned = false;
    FMap* MyMap = nullptr;
};

