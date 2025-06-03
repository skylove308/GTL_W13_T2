#include "StreetLight.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"

AStreetLight::AStreetLight()
{
    StreetLightMesh = AddComponent<UStaticMeshComponent>("StreetLightMesh");
    RootComponent = StreetLightMesh;
}

void AStreetLight::Initialize(FVector SpawnWorldLocation)
{
    StreetLightMesh->SetWorldLocation(SpawnWorldLocation);
    StreetLightMesh->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/StreetLight/Light.obj"));
    StreetLightMesh->SetWorldRotation(FRotator(0.0f, 180.0f, 0.0f));
    StreetLightMesh->SetWorldScale3D(FVector(100.0f, 100.0f, 100.0f));
    StreetLightMesh->bSimulate = true;
    StreetLightMesh->RigidBodyType = ERigidBodyType::STATIC;
}

void AStreetLight::BeginPlay()
{
    AActor::BeginPlay();

    StreetLightMesh->CreatePhysXGameObject();
}

