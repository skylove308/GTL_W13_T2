#include "Road.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"

ARoad::ARoad()
{
    UStaticMeshComponent* StaticMeshComp = AddComponent<UStaticMeshComponent>("Road");
    StaticMeshComp->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Road/road.obj"));
    RootComponent = StaticMeshComp;
    RootComponent->SetWorldRotation(FRotator(0.0f, 90.0f, 90.0f));
    RootComponent->SetWorldScale3D(FVector(100.0f, 100.0f, 100.0f));
}
