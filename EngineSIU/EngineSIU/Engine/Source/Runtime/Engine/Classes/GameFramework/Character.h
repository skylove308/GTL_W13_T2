#pragma once
#include "Pawn.h"

class ACharacter : public APawn
{
    DECLARE_CLASS(ACharacter, APawn)
public:
    ACharacter() = default;
    virtual UObject* Duplicate(UObject* InOuter) override;
    
    virtual void PostSpawnInitialize() override { Super::PostSpawnInitialize(); }
    virtual void Tick(float DeltaTime) override { Super::Tick(DeltaTime); }

    virtual void SetupInputComponent(UInputComponent* PlayerInputComponent) override { }
};
