#include "Road.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"

ARoad::ARoad()
{
    UStaticMeshComponent* StaticMeshComp = AddComponent<UStaticMeshComponent>("CarMesh");
    StaticMeshComp->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Primitives/CubePrimitive.Obj"));
    RootComponent = StaticMeshComp;
}
