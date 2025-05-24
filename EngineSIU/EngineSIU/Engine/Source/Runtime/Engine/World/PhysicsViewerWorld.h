#pragma once
#include "World.h"

class UPhysicsAssetViewerWorld : public UWorld
{
    DECLARE_CLASS(UPhysicsAssetViewerWorld, UWorld)
public:
    UPhysicsAssetViewerWorld() = default;

    static UPhysicsAssetViewerWorld* CreateWorld(UObject* InOuter,
        const EWorldType InWorldType = EWorldType::PhysicsAssetViewer,
        const FString& InWorldName = "PhysicsAssetViewerWorld"
    );

    //void SetPhysicsAsset(UPhysicsAssetComponent* Component)
    //{
    //    PhysicsAsset = Component;
    //}

    void Tick(float DeltaTime) override;

private:
    //UPhysicsAssetComponent* PhysicsAsset = nullptr;
};

