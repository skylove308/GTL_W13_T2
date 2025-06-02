#include "SkySphereActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"
#include "Editor/LevelEditor/SLevelEditor.h"
#include "Editor/UnrealEd/EditorViewportClient.h"

ASkySphere::ASkySphere()
{

}

void ASkySphere::PostSpawnInitialize()
{
    UStaticMesh* SkySphereMesh = FObjManager::GetStaticMesh(L"Contents/Skysphere/skysphere.obj");
    SkySphereMeshComp = AddComponent<UStaticMeshComponent>(L"SkySphereMeshComp");
    SkySphereMeshComp->SetStaticMesh(SkySphereMesh);
    RootComponent = SkySphereMeshComp;

    SetActorScale(FVector(50000.0f, 50000.0f, 50000.0f));
}

UObject* ASkySphere::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    if (!NewActor) return nullptr;

    NewActor->SkySphereMeshComp = NewActor->GetComponentByClass<UStaticMeshComponent>();
    return NewActor;
}

void ASkySphere::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    auto VPC = GEngineLoop.GetLevelEditor()->GetActiveViewportClient().get();
    if (VPC)
    {
        const FVector CameraLocation = VPC->GetCameraLocation();
        SetActorLocation(CameraLocation);
    }
}
