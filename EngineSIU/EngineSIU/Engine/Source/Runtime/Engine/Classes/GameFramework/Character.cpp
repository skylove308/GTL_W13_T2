#include "Character.h"

#include "Engine/Engine.h"
#include "PhysicsManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"

ACharacter::ACharacter()
{
    CapsuleComponent = AddComponent<UCapsuleComponent>("CapsuleComponent");
    CapsuleComponent->InitCapsuleSize(42.f, 96.f);
    CapsuleComponent->bLockXRotation = true;
    CapsuleComponent->bLockYRotation = true;
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
    if (Value == 0.0f) return;

    if (Speed <= MaxSpeed)
    {
        Speed += 0.01f;
    }
    else
    {
        Speed = MaxSpeed;
    }

    FVector Forward = GetActorForwardVector() * Speed * Value;
    FVector NewLocation = GetActorLocation() + Forward;
    SetActorLocation(NewLocation);

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
    if (Value == 0.0f) return;

    if (Speed <= MaxSpeed)
    {
        Speed += 0.01f;
    }
    else
    {
        Speed = MaxSpeed;
    }

    FVector Right = GetActorRightVector() * Speed * Value;
    FVector NewLocation = GetActorLocation() + Right;
    SetActorLocation(NewLocation);

    if (Value >= 0.0f)
    {
        MeshComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    }
    else
    {
        MeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }
}
