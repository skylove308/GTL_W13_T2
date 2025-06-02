#include "Road.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"
#include "Lua/LuaScriptComponent.h"
#include "Engine/Engine.h"
#include "World/World.h"
#include "Actors/Cube.h"


ARoad::ARoad()
{
    RoadMesh = AddComponent<UStaticMeshComponent>("Road");
    RootComponent = RoadMesh;
}

void ARoad::Initialize(ERoadState RoadState, FVector SpawnWorldLocation)
{
    CurrentRoadState = RoadState;
    RoadMesh->SetWorldLocation(SpawnWorldLocation);
    RoadMesh->SetWorldRotation(FRotator(0.0f, 0.0f, 90.0f));
    RoadMesh->SetWorldScale3D(FVector(30.0f, 30.0f, 1000.0f));

    if (RoadState == ERoadState::Safe)
    {
        RoadMesh->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Road/Road.obj"));
        RoadMesh->bSimulate = true;
        RoadMesh->RigidBodyType = ERigidBodyType::STATIC;
    }
    else if (RoadState == ERoadState::Car)
    {
        RoadMesh->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Road/Road.obj"));
        RoadMesh->bSimulate = true;
        RoadMesh->RigidBodyType = ERigidBodyType::STATIC;
    }


}

void ARoad::BeginPlay()
{
    Super::BeginPlay();
}

void ARoad::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    OnOverlappedRoad(DeltaTime);
}

void ARoad::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (EndPlayReason == EEndPlayReason::WorldTransition)
    {
        for (int i = 0; i < RoadMesh->GetNumMaterials(); i++)
        {
            FVector EmissiveColor = FVector(0.0f, 0.0f, 0.0f);
            RoadMesh->GetMaterial(i)->SetEmissive(EmissiveColor);
        }
    }
}

UObject* ARoad::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewActor->CurrentRoadState = CurrentRoadState;
    NewActor->CurrentRoadTime = CurrentRoadTime;
    NewActor->SafeJoneTime = SafeJoneTime;
    NewActor->WarningJoneTime = WarningJoneTime;
    NewActor->bIsOverlapped = bIsOverlapped;
    NewActor->RoadMesh = NewActor->GetComponentByClass<UStaticMeshComponent>();

    return NewActor;
}


void ARoad::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(ARoad, sol::bases<AActor>(),
        "CurrentRoadState", sol::property(&ThisClass::GetCurrentRoadState, &ThisClass::SetCurrentRoadState),
        "CurrentRoadTime", sol::property(&ThisClass::GetCurrentRoadTime, &ThisClass::SetCurrentRoadTime),
        "SafeJoneTime", sol::property(&ThisClass::GetSafeJoneTime, &ThisClass::SetSafeJoneTime),
        "WarningJoneTime", sol::property(&ThisClass::GetWarningJoneTime, &ThisClass::SetWarningJoneTime),
        "DestroyRoad", &ThisClass::DestroyRoad
    )
}


bool ARoad::BindSelfLuaProperties()
{
    if (!Super::BindSelfLuaProperties())
        return false;

    sol::table& LuaTable = LuaScriptComponent->GetLuaSelfTable();
    if (!LuaTable.valid())
    {
        return false;
    }

    LuaTable["this"] = this;
    LuaTable["Name"] = *GetName();

    return true;
}


void ARoad::OnOverlappedRoad(float DeltaTime)
{
    if (!bIsOverlapped)
    {
        CurrentRoadState = ERoadState::Safe;
        if (CurrentRoadTime > SafeJoneTime)
        {
            OnNoRed.Broadcast();
        }
        CurrentRoadTime = 0.0f;

        return;
    }

    if (CurrentRoadState == ERoadState::Safe)
    {
        CurrentRoadTime += DeltaTime;
        if (CurrentRoadTime >= SafeJoneTime)
        {
            CurrentRoadState = ERoadState::Warning;
        }
    }
    else if (CurrentRoadState == ERoadState::Warning)
    {
        OnRed.Broadcast();

        CurrentRoadTime += DeltaTime;
        if (CurrentRoadTime >= SafeJoneTime + WarningJoneTime)
        {
            CurrentRoadState = ERoadState::Danger;
            CurrentRoadTime = 0.0f;
        }
    }
    else if (CurrentRoadState == ERoadState::Danger)
    {
        OnDeath.Broadcast();
    }


}

void ARoad::DestroyRoad()
{
    this->Destroy();
}
