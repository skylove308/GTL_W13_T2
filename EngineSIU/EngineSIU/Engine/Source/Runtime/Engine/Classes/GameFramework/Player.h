#pragma once
#include "Character.h"
#include "UObject/ObjectMacros.h"

class UCameraComponent;
class USkeletalMeshComponent;

class APlayer : public ACharacter
{
    DECLARE_CLASS(APlayer, ACharacter)
public:
    APlayer() = default;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void PostSpawnInitialize() override;
    virtual void Tick(float DeltaTime) override;
private:
    void MoveForward(float DeltaTime);
    void MoveRight(float DeltaTime);
    void MoveUp(float DeltaTime);

    void RotateYaw(float DeltaTime);
    void RotatePitch(float DeltaTime);

    virtual void SetupInputComponent(UInputComponent* PlayerInputComponent) override;

    FName Socket = "jx_c_camera";
    
    USkeletalMeshComponent* SkeletalMeshComponent = nullptr;
    UCameraComponent* CameraComponent = nullptr;

    float MoveSpeed = 100.0f; // 이동 속도
    float RotationSpeed = 0.1f; // 회전 속도
};
