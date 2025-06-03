#include "StreetLight.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Light/SpotLightComponent.h"
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

void AStreetLight::CreateSpotLight()
{
    if (SpotLight != nullptr)
        return;
    
    SpotLight = AddComponent<USpotLightComponent>();
    SpotLight->AttachToComponent(RootComponent);
    SpotLight->SetRelativeLocation(FVector(127.78f, -1.02f, 440.61f));
    SpotLight->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));
    SpotLight->SetIntensity(1000000.0f);
    SpotLight->SetRadius(800.0f);
    SpotLight->SetInnerDegree(24.0f);
    SpotLight->SetOuterDegree(30.0f);
}

void AStreetLight::BeginPlay()
{
    AActor::BeginPlay();

    StreetLightMesh->CreatePhysXGameObject();
}

