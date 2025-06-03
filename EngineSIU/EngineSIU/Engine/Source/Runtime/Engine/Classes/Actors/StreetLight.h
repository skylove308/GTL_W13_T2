#pragma once
#include "GameFramework/Actor.h"

class USpotLightComponent;
class UStaticMeshComponent;

class AStreetLight : public AActor
{
    DECLARE_CLASS(AStreetLight, AActor)

public:
    AStreetLight();

    virtual void BeginPlay() override;

    void Initialize(FVector SpawnWorldLocation = FVector(0.0f, 0.0f, 0.0f));
    void CreateSpotLight();

    USpotLightComponent* GetSpotLight() const { return SpotLight; }
private:
    UStaticMeshComponent* StreetLightMesh = nullptr;
    USpotLightComponent* SpotLight = nullptr;
};
