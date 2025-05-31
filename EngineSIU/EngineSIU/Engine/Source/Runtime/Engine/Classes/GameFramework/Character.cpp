#include "Character.h"

#include "Engine/Engine.h"
#include "PhysicsManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include <PxRigidActor.h>

ACharacter::ACharacter()
{
    CapsuleComponent = AddComponent<UCapsuleComponent>("CapsuleComponent");
    CapsuleComponent->InitCapsuleSize(42.f, 96.f);
    CapsuleComponent->bSimulate = true;
    CapsuleComponent->bApplyGravity = true;
    CapsuleComponent->bLockXRotation = true;
    CapsuleComponent->bLockYRotation = true;
    CapsuleComponent->RigidBodyType = ERigidBodyType::DYNAMIC;
    RootComponent = CapsuleComponent;

    MeshComponent = AddComponent<USkeletalMeshComponent>("SkeletalMeshComponent");
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetAnimClass(UClass::FindClass(FName("UMyAnimInstance")));

    MovementComponent = AddComponent<UCharacterMovementComponent>("CharacterMovementComponent");
    MovementComponent->UpdatedComponent = CapsuleComponent;

    CameraComponent = AddComponent<UCameraComponent>("CameraComponent");
    CameraComponent->SetupAttachment(RootComponent);
}

void ACharacter::BeginPlay()
{
    APawn::BeginPlay();
}

UObject* ACharacter::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    if (!NewActor) return nullptr;

    NewActor->CapsuleComponent = NewActor->GetComponentByClass<UCapsuleComponent>();
    NewActor->MeshComponent = NewActor->GetComponentByClass<USkeletalMeshComponent>();
    NewActor->MovementComponent = NewActor->GetComponentByClass<UCharacterMovementComponent>();
    NewActor->CameraComponent = NewActor->GetComponentByClass<UCameraComponent>();

    if (NewActor->MovementComponent && NewActor->CapsuleComponent)
    {
        NewActor->MovementComponent->UpdatedComponent = NewActor->CapsuleComponent;
    }
    
    return NewActor;
}

void ACharacter::Tick(float DeltaTime)
{
    APawn::Tick(DeltaTime);

    // 물리 결과 동기화
    // if (bPhysXInitialized &&  PhysXActor)
    // {
    //     PxTransform PxTr = PhysXActor->getGlobalPose();
    //     SetActorLo
    //
    //
    //     cation(FVector(PxTr.p.x, PxTr.p.y, PxTr.p.z));
    // }
}

void ACharacter::MoveForward(float Value)
{
    if (Value == 0.0f)
    {
        CurrentForce = 0.0f;
        return;
    }

    physx::PxRigidDynamic* PxCharActor = static_cast<physx::PxRigidDynamic*>(CapsuleComponent->BodyInstance->RigidActorSync);
    if (PxCharActor == nullptr)
        return;

    CurrentForce = FMath::Min(CurrentForce + ForceIncrement, MaxForce);
    FVector Forward = GetActorForwardVector().GetSafeNormal();
    physx::PxVec3 PushForce(
        Forward.X * CurrentForce * Value,
        Forward.Y * CurrentForce * Value,
        Forward.Z * CurrentForce * Value
    );

    // PushForce: PhysX 액터에 가할 힘 벡터 (PxVec3) — 뉴턴 단위, 월드 좌표 기준으로 적용됩니다.
    // physx::PxForceMode::eFORCE: 지속적인 힘 모드. 매 시뮬레이션 스텝마다 힘(force) / 질량(mass)에 따라 가속도가 계산되어 적용됩니다.
    // /*autowake=*/ true: 슬립 상태인 리지드 바디라도 강제로 깨워서 즉시 물리 시뮬레이션에 반영하도록 설정합니다.
    PxCharActor->addForce(PushForce, physx::PxForceMode::eFORCE, /*autowake=*/ true);

    if (Value >= 0.0f)
    {
        MeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
    }
    else
    {
        MeshComponent->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
    }
}

void ACharacter::MoveRight(float Value)
{
    if (Value == 0.0f)
    {
        CurrentForce = 0.0f;
        return;
    }

    physx::PxRigidDynamic* PxCharActor = static_cast<physx::PxRigidDynamic*>(CapsuleComponent->BodyInstance->RigidActorSync);
    if (PxCharActor == nullptr)
        return;

    CurrentForce = FMath::Min(CurrentForce + ForceIncrement, MaxForce);

    FVector Right = GetActorRightVector().GetSafeNormal();
    physx::PxVec3 PushForce(
        Right.X * CurrentForce * Value,
        Right.Y * CurrentForce * Value,
        Right.Z * CurrentForce * Value
    );

    PxCharActor->addForce(PushForce, physx::PxForceMode::eFORCE, true);

    if (Value >= 0.0f)
    {
        MeshComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    }
    else
    {
        MeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }
}
