#pragma once
#include "ActorComponent.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"


class UPrimitiveComponent;

class UMovementComponent : public UActorComponent
{
    DECLARE_CLASS(UMovementComponent, UActorComponent)

public:
    UMovementComponent() = default;
    virtual ~UMovementComponent() override = default;

    UPrimitiveComponent* UpdatedComponent;
};
