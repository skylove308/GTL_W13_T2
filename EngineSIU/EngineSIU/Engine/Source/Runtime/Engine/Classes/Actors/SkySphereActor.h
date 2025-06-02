#pragma once
#include "GameFramework/Actor.h"

class UStaticMesh;
class UStaticMeshComponent;
class ASkySphere : public AActor
{
    DECLARE_CLASS(ASkySphere, AActor)
public:
    ASkySphere();
    virtual void PostSpawnInitialize() override;
    virtual UObject* Duplicate(UObject* InOuter);
    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY_WITH_FLAGS(EditAnywhere, UStaticMeshComponent*, SkySphereMeshComp, = nullptr)
};
