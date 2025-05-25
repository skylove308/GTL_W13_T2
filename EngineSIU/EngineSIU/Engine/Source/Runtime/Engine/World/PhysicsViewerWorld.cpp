#include "PhysicsViewerWorld.h"

UPhysicsAssetViewerWorld* UPhysicsAssetViewerWorld::CreateWorld(UObject* InOuter, const EWorldType InWorldType, const FString& InWorldName)
{
    UPhysicsAssetViewerWorld* NewWorld = FObjectFactory::ConstructObject<UPhysicsAssetViewerWorld>(InOuter);
    NewWorld->WorldName = InWorldName;
    NewWorld->WorldType = InWorldType;
    NewWorld->InitializeNewWorld();

    return NewWorld;
}

void UPhysicsAssetViewerWorld::Tick(float DeltaTime)
{
    UWorld::Tick(DeltaTime);
}
