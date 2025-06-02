#pragma once
#include <PxRigidDynamic.h>
#include "Pawn.h"
#include "PhysicsManager.h"
#include "Components/CapsuleComponent.h"

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
    void Stop();

    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.

public:
    virtual void OnCollisionEnter(UPrimitiveComponent* HitComponent, UPrimitiveComponent* OtherComp, const FHitResult& Hit) override;

    float GetSpeed();
    void SetSpeed(float NewVelocity);

    bool GetIsRunning() { return bIsRunning; }
    void SetIsRunning(bool bInIsRunning) { bIsRunning = bInIsRunning; }

    bool GetIsDead() { return bIsDead; }
    void SetIsDead(bool bInIsDead) { bIsDead = bInIsDead; }

public:
    bool bIsRunning = false;
    bool bIsDead = false;
    bool bIsStop = true;

    bool bCameraEffect = false;
    bool bSwitchCamera = false;
    float CurrentDeathCameraTransitionTime = 3.0f;
    float CurrentDeathLetterBoxTransitionTime = 2.0f;

    const float DeathCameraTransitionTime = 0.5f;
    const float DeathLetterBoxTransitionTime = 2.0f;
    
    float CurrentForce = 0.0f;
    float TotalForce = 0.0f;
    
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, ForceIncrement,  = 1000.0f)
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, MaxForce,  = 100000.0f)
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, MovementForceMultiplier,  = 1.0f)
 
private:
    virtual void BeginPlay() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void Tick(float DeltaTime) override;

    void DoCameraEffect(float DeltaTime);
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, ImpulseScale, = 100000.f)
};
