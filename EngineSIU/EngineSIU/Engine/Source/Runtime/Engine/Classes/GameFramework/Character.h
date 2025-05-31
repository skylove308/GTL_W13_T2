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

    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.

    float GetSpeed() const { return Speed; }
    void SetSpeed(float NewSpeed) { Speed = NewSpeed; }

    float GetMaxSpeed() const { return MaxSpeed; }
    void SetMaxSpeed(float NewMaxSpeed) { MaxSpeed = NewMaxSpeed; }

    float Speed = 6.0f;
    float VelocityZ = 0.0f;

    float MaxSpeed = 12.0f;

private:
    virtual void BeginPlay() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void Tick(float DeltaTime) override;
};
