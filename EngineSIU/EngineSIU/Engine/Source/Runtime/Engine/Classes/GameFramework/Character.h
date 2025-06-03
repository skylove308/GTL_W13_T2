#pragma once
#include <PxRigidDynamic.h>
#include "Pawn.h"
#include "PhysicsManager.h"
#include "Components/CapsuleComponent.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class UCharacterMovementComponent;
class UCameraComponent;
class AGameManager;

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
    void ApplyMovementForce(const FVector& Direction, float Scale);
    void Stop();
    void RotateCharacterMesh(float DeltaTime);


    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.

public:
    virtual void OnCollisionEnter(UPrimitiveComponent* HitComponent, UPrimitiveComponent* OtherComp, const FHitResult& Hit) override;
    virtual void OnCollisionExit(UPrimitiveComponent* HitComponent, UPrimitiveComponent* OtherComp) override;

    float GetSpeed();
    void SetSpeed(float NewVelocity);

    bool GetIsRunning() { return bIsRunning; }
    void SetIsRunning(bool bInIsRunning) { bIsRunning = bInIsRunning; }

    bool GetIsDead() { return bIsDead; }
    void SetIsDead(bool bInIsDead) { bIsDead = bInIsDead; }

    void BindInput();
    void UnbindInput();

public:
    bool bIsRunning = false;
    bool bIsDead = false;
    bool bIsStop = true;

    bool bCameraEffect = false;
    bool bSwitchCamera = false;
    float CurrentDeathCameraTransitionTime = 3.0f;
    float CurrentDeathLetterBoxTransitionTime = 2.0f;
    
    float CurrentForce = 0.0f;
    float TotalForce = 0.0f;
    
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, DeathCameraTransitionTime,  = 3.0f)
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, DeathLetterBoxTransitionTime,  = 2.0f)

    UPROPERTY_WITH_FLAGS(EditAnywhere, float, WalkForce, = 200000.f)
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, RunForce, = 400000.f)

    UPROPERTY_WITH_FLAGS(EditAnywhere, float, WalkMaxSpeed, = 300.f)
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, RunMaxSpeed, = 600.f)

    UPROPERTY_WITH_FLAGS(EditAnywhere, float, InputLinearDamping, = 2.0f)
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, NoInputLinearDamping, = 7.0f)

    UPROPERTY_WITH_FLAGS(EditAnywhere, float, MeshRotationSpeed, = 10.0f)

    AGameManager* GameManager = nullptr;

                
private:
    virtual void BeginPlay() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

    void DoCameraEffect(float DeltaTime);

    UPROPERTY_WITH_FLAGS(EditAnywhere, float, ImpulseScale, = 100000.f)
    UPROPERTY_WITH_FLAGS(EditAnywhere, class UParticleSystem*, ExplosionParticle, = nullptr)
};
