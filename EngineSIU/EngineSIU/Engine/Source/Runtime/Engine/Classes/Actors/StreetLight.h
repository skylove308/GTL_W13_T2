#pragma once
#include "GameFramework/Actor.h"

class UStaticMeshComponent;

class AStreetLight : public AActor
{
    DECLARE_CLASS(AStreetLight, AActor)

public:
    AStreetLight();

    void Initialize(FVector SpawnWorldLocation = FVector(0.0f, 0.0f, 0.0f));

    virtual void BeginPlay() override;

private:
    UStaticMeshComponent* StreetLightMesh = nullptr;
};
