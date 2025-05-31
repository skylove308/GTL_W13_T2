#pragma once
#include <PxRigidDynamic.h>
#include "Pawn.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class UCharacterMovementComponent;
class UCameraComponent;

class ACharacter : public APawn
{
    DECLARE_CLASS(ACharacter, APawn)

public:
    ACharacter();

    UCapsuleComponent* CapsuleComponent = nullptr;
    USkeletalMeshComponent* MeshComponent = nullptr;
    UCharacterMovementComponent* MovementComponent = nullptr;
    UCameraComponent* CameraComponent = nullptr;

    void MoveForward(float Value);
    void MoveRight(float Value);

    float CurrentForce = 0.0f;
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, ForceIncrement,  = 1000.0f)
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, MaxForce,  = 100000.0f)
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, MovementForceMultiplier,  = 1.0f)
                
private:
    virtual void BeginPlay() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void Tick(float DeltaTime) override;
};
