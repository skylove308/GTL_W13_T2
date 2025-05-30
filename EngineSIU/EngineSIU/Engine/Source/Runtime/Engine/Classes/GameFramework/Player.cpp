#include "Player.h"

#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "World/World.h"

UObject* APlayer::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewActor->Socket = Socket;
    
    return NewActor;
}

void APlayer::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();
    
    RootComponent = AddComponent<USceneComponent>();

    CameraComponent = AddComponent<UCameraComponent>();
    CameraComponent->SetupAttachment(RootComponent);
}

void APlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (SkeletalMeshComponent)
    {
        const FTransform SocketTransform = SkeletalMeshComponent->GetSocketTransform(Socket);
        SetActorRotation(SocketTransform.GetRotation().Rotator());
        SetActorLocation(SocketTransform.GetTranslation());
    }
}

void APlayer::SetupInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupInputComponent(PlayerInputComponent);
    // 카메라 조작용 축 바인딩
    if (PlayerInputComponent)
    {
        PlayerInputComponent->BindAction("W", [this](float DeltaTime) { MoveForward(DeltaTime); });
        PlayerInputComponent->BindAction("S", [this](float DeltaTime) { MoveForward(-DeltaTime); });
        PlayerInputComponent->BindAction("A", [this](float DeltaTime) { MoveRight(-DeltaTime); });
        PlayerInputComponent->BindAction("D", [this](float DeltaTime) { MoveRight(DeltaTime); });
        PlayerInputComponent->BindAction("E", [this](float DeltaTime) { MoveUp(DeltaTime); });
        PlayerInputComponent->BindAction("Q", [this](float DeltaTime) { MoveUp(-DeltaTime); });

        PlayerInputComponent->BindAxis("Turn", [this](float DeltaTime) { RotateYaw(DeltaTime); });
        PlayerInputComponent->BindAxis("LookUp", [this](float DeltaTime) { RotatePitch(DeltaTime); });
    }
}

void APlayer::MoveForward(float DeltaTime)
{
    UE_LOG(ELogLevel::Display, "MoveForward Execute");
    FVector Delta = GetActorForwardVector() * MoveSpeed * DeltaTime;
    SetActorLocation(GetActorLocation() + Delta);
}

void APlayer::MoveRight(float DeltaTime)
{
    FVector Delta = GetActorRightVector() * MoveSpeed * DeltaTime;
    SetActorLocation(GetActorLocation() + Delta);
}

void APlayer::MoveUp(float DeltaTime)
{
    FVector Delta = GetActorUpVector() * MoveSpeed * DeltaTime;
    SetActorLocation(GetActorLocation() + Delta);
}

void APlayer::RotateYaw(float DeltaTime)
{
    UE_LOG(ELogLevel::Display, "RotateYaw Execute");
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += DeltaTime * RotationSpeed; // Yaw 회전 속도
    SetActorRotation(NewRotation);
}

void APlayer::RotatePitch(float DeltaTime)
{
    UE_LOG(ELogLevel::Display, "RotatePitch Execute");
    FRotator NewRotation = GetActorRotation();
    NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch - DeltaTime*RotationSpeed, -89.0f, 89.0f);
    SetActorRotation(NewRotation);
}
