#include "Character.h"

#include "Engine/Engine.h"
#include "PhysicsManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CharacterMovementComponent.h"

ACharacter::ACharacter()
{
    CapsuleComponent = AddComponent<UCapsuleComponent>("CapsuleComponent");
    CapsuleComponent->InitCapsuleSize(42.f, 96.f);
    CapsuleComponent->bLockXRotation = true;
    CapsuleComponent->bLockYRotation = true;
    RootComponent = CapsuleComponent;

    MeshComponent = AddComponent<USkeletalMeshComponent>("SkeletalMeshComponent");
    MeshComponent->SetupAttachment(RootComponent);

    MovementComponent = AddComponent<UCharacterMovementComponent>("CharacterMovementComponent");
    MovementComponent->UpdatedComponent = CapsuleComponent;
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
