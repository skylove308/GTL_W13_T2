#pragma once
#include "MovementComponent.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"


class UPawnMovementComponent : public UMovementComponent
{
    DECLARE_CLASS(UPawnMovementComponent, UMovementComponent)

public:
    UPawnMovementComponent() = default;
    virtual ~UPawnMovementComponent() override = default;
    
};
