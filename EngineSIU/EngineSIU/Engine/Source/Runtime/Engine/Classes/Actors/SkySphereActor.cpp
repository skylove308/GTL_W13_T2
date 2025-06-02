#include "SkySphereActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"

ASkySphere::ASkySphere()
{
    SkySphereMesh = FObjManager::GetStaticMesh(L"Contents/Sphere.obj");
    //SkySphereMesh->GetRenderData()->
    RootComponent = nullptr;
}

UObject* ASkySphere::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    if (!NewActor) return nullptr;

    return NewActor;
}

void ASkySphere::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
