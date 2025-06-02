#pragma once
#include "GameFramework/Actor.h"

class UStaticMesh;
class ASkySphere : public AActor
{
    DECLARE_CLASS(ASkySphere, AActor)
public:
    ASkySphere();

    virtual UObject* Duplicate(UObject* InOuter);
    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY_WITH_FLAGS(EditAnywhere, UStaticMesh*, SkySphereMesh, = nullptr)
};
