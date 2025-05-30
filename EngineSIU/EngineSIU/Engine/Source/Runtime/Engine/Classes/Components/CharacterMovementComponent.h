#pragma once
#include "Components/PawnMovementComponent.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UCharacterMovementComponent : public UPawnMovementComponent
{
    DECLARE_CLASS(UCharacterMovementComponent, UPawnMovementComponent)

public:
    UCharacterMovementComponent();

    virtual void TickComponent(float DeltaTime) override;
    
};
