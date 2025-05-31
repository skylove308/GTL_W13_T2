#pragma once
#include "GameFramework/Actor.h"

class ACar : public AActor
{
    DECLARE_CLASS(ACar, AActor)
public:
    ACar();
    virtual void BeginPlay() override;

    UPROPERTY_WITH_FLAGS(EditAnywhere, FVector, InitialVelocity)
};
