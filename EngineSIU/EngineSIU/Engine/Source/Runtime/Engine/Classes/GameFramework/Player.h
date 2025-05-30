#pragma once
#include "Character.h"
#include "UObject/ObjectMacros.h"

class APlayer : public ACharacter
{
    DECLARE_CLASS(APlayer, ACharacter)
public:
    APlayer() = default;
    
private:
    void MoveForward(float DeltaTime);
    void MoveRight(float DeltaTime);
    void MoveUp(float DeltaTime);

    void RotateYaw(float DeltaTime);
    void RotatePitch(float DeltaTime);

    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    float MoveSpeed = 100.0f; // 이동 속도
    float RotationSpeed = 0.1f; // 회전 속도
};
