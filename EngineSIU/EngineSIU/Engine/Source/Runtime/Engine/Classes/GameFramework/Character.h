#pragma once
#include <PxRigidDynamic.h>
#include "Pawn.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class UCharacterMovementComponent;

class ACharacter : public APawn
{
    DECLARE_CLASS(ACharacter, APawn)

public:
    ACharacter();

    UCapsuleComponent* CapsuleComponent;
    USkeletalMeshComponent* MeshComponent;
    UCharacterMovementComponent* MovementComponent;

private:
    virtual void BeginPlay() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void Tick(float DeltaTime) override;
};
