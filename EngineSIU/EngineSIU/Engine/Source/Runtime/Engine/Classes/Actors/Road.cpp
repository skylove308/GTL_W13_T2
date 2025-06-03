#include "Road.h"

#include "StreetLight.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"
#include "Lua/LuaScriptComponent.h"
#include "Engine/Engine.h"
#include "World/World.h"
#include "Actors/Cube.h"
#include "Actors/Car.h"
#include "Actors/GameManager.h"
#include "Components/Light/SpotLightComponent.h"
#include "Engine/Contents/Maps/MapModule.h"


ARoad::ARoad()
{
    RoadMesh = AddComponent<UStaticMeshComponent>("Road");
    RootComponent = RoadMesh;
}

void ARoad::Initialize(ERoadState RoadState, FMap* Map, FVector SpawnWorldLocation)
{
    CurrentRoadState = RoadState;
    RoadMesh->SetWorldLocation(SpawnWorldLocation);
    MyMap = Map;

    if (RoadState == ERoadState::Safe)
    {
        RoadMesh->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Road/Road2/Ground.obj"));
        RoadMesh->SetWorldRotation(FRotator(0.0f, 90.0f, 0.0f));
        RoadMesh->SetWorldScale3D(FVector(10.0f, 10.0f, 10.0f));
        RoadMesh->bSimulate = true;
        RoadMesh->RigidBodyType = ERigidBodyType::STATIC;
    }
    else if (RoadState == ERoadState::Car)
    {
        RoadMesh->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Road/Road2/Road2.obj"));
        RoadMesh->SetWorldRotation(FRotator(0.0f, 90.0f, 0.0f));
        RoadMesh->SetWorldScale3D(FVector(10.0, 10.0, 10.0));
        RoadMesh->bSimulate = true;
        RoadMesh->RigidBodyType = ERigidBodyType::STATIC;
    }
}

void ARoad::BeginPlay()
{
    Super::BeginPlay();

    RoadMesh->CreatePhysXGameObject();

    auto Actors = GEngine->ActiveWorld->GetActiveLevel()->Actors;
    for (auto Actor : Actors)
    {
        if (Actor->IsA<AGameManager>())
        {
            GameManager = Cast<AGameManager>(Actor);
            break;
        }
    }
}

void ARoad::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    OnOverlappedRoad(DeltaTime);

    int RandNum = FMath::RandHelper(500);
    int DirectionNum = FMath::RandHelper(2);
    if (CurrentRoadState == ERoadState::Car && RandNum == 0 && !bIsCarOnRoad)
    {
        CurrentCar = GEngine->ActiveWorld->SpawnActor<ACar>();
        if (DirectionNum == 0)
        {
            ECarType CarType = CurrentCar->GetCarType();
            switch (CarType)
            {
            case ECarType::Benz:
                CurrentCar->SetActorLocation(FVector(GetActorLocation().X, 8000.0f, 305.0f));
                break;
            case ECarType::RangeRover:
                CurrentCar->SetActorLocation(FVector(GetActorLocation().X, 8000.0f, 305.0f));
                break;
            case ECarType::Truck:
                CurrentCar->SetActorLocation(FVector(GetActorLocation().X, 8000.0f, 305.0f));
                break;
            case ECarType::Train:
                CurrentCar->SetActorLocation(FVector(GetActorLocation().X, 8000.0f, 350.0f));
                break;
            }

            CurrentCar->SetSpawnDirectionRight(true);
        }
        else
        {
            ECarType CarType = CurrentCar->GetCarType();
            switch (CarType)
            {
            case ECarType::Benz:
                CurrentCar->SetActorLocation(FVector(GetActorLocation().X, -8000.0f, 305.0f));
                break;
            case ECarType::RangeRover:
                CurrentCar->SetActorLocation(FVector(GetActorLocation().X, -8000.0f, 305.0f));
                break;
            case ECarType::Truck:
                CurrentCar->SetActorLocation(FVector(GetActorLocation().X, -8000.0f, 305.0f));
                break;
            case ECarType::Train:
                CurrentCar->SetActorLocation(FVector(GetActorLocation().X, -8000.0f, 350.0f));
                break;
            }

            FRotator CarRotation = CurrentCar->GetRootComponent()->GetComponentRotation();
            CurrentCar->GetRootComponent()->SetWorldRotation(FRotator(CarRotation.Pitch, -CarRotation.Yaw, CarRotation.Roll));
            CurrentCar->SetSpawnDirectionRight(false);
        }

        Cast<UPrimitiveComponent>(CurrentCar->GetRootComponent())->CreatePhysXGameObject();

        bIsCarOnRoad = true;
    }

    if (bIsCarOnRoad && CarOnRoadTime < 10.0f)
    {
        CarOnRoadTime += DeltaTime;
    }

    if (bIsCarOnRoad && CarOnRoadTime >= 10.0f)
    {
        CarOnRoadTime = 0.0f;
        bIsCarOnRoad = false;
    }
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

void ARoad::TurnOnStreetLights()
{
    if (MyMap == nullptr)
        return;
    
    for (AStreetLight* StreetLight : MyMap->StreetLights)
    {
        StreetLight->CreateSpotLight();
    }
}

void ARoad::OnOverlappedRoad(float DeltaTime)
{
    if (!bIsOverlapped && CurrentRoadState != ERoadState::Car)
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
        if (!bIsFirstTimeOnRoad)
        {
            bIsFirstTimeOnRoad = true;
            int CurrentScore = GameManager->GetScore();
            GameManager->SetScore(CurrentScore + 1);
            GameManager->SpawnMap();
            GameManager->DestroyMap();
        }

        // 첫 로드에 있는 경우에는 경고 상태로 가지 않도록 함
        if (GameManager->GetMapModule()->GetMaps().front()->Roads[0] == this) 
        {
            return;
        }

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
