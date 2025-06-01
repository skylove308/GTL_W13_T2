#include "Character.h"
#include "Engine/Engine.h"
#include "PhysicsManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"
#include "Lua/LuaScriptComponent.h"
#include "Lua/LuaScriptManager.h"
#include "World/World.h"
#include "Actors/Car.h"

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

    CameraComponent->FollowMainPlayer();
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
    if (GetActorLocation().X > 100 && !bSwitchCamera)
    {
        for (auto Actor : GEngine->ActiveWorld->GetActiveLevel()->Actors)
        {
            if (ACar* Car = Cast<ACar>(Actor))
            {
                FViewTargetTransitionParams Params;
                Params.BlendTime = 3.0f; // 카메라 전환 시간
                {
                    GEngine->ActiveWorld->GetPlayerController()->SetViewTarget(Car, Params);
                    GEngine->ActiveWorld->GetPlayerController()->Possess(Car);
                }
                bSwitchCamera = true;
                break;
            }
        }
    }
}

void ACharacter::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(ACharacter, sol::bases<AActor>(),
        "Speed", sol::property(&ThisClass::GetSpeed, &ThisClass::SetSpeed),
        "MaxSpeed", sol::property(&ThisClass::GetMaxSpeed, &ThisClass::SetMaxSpeed)
    )
}

bool ACharacter::BindSelfLuaProperties()
{
    Super::BindSelfLuaProperties();

    sol::table& LuaTable = LuaScriptComponent->GetLuaSelfTable();
    if (!LuaTable.valid())
    {
        return false;
    }

    // 자기 자신 등록.
    // self에 this를 하게 되면 내부에서 임의로 Table로 바꿔버리기 때문에 self:함수() 형태의 호출이 불가능.
    // 자기 자신 객체를 따로 넘겨주어야만 AActor:GetName() 같은 함수를 실행시켜줄 수 있다.
    LuaTable["this"] = this;
    LuaTable["Name"] = *GetName(); // FString 해결되기 전까지 임시로 Table로 전달.
    // 이 아래에서 또는 하위 클래스 함수에서 멤버 변수 등록.

    return true;
}

void ACharacter::MoveForward(float Value)
{
    if (Value == 0.0f) return;

    //if (Speed <= MaxSpeed)
    //{
    //    Speed += 0.01f;
    //}
    //else
    //{
    //    Speed = MaxSpeed;
    //}

    if (bIsRunning)
    {
        Speed = 12.0f;
    }
    else
    {
        Speed = 7.0f;
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

    //if (Speed <= MaxSpeed)
    //{
    //    Speed += 0.01f;
    //}
    //else
    //{
    //    Speed = MaxSpeed;
    //}

    if (bIsRunning)
    {
        Speed = 12.0f;
    }
    else
    {
        Speed = 7.0f;
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

void ACharacter::RunFast(bool bInIsRunning)
{
    if (bInIsRunning)
    {
        Speed = MaxSpeed * 2.0f; // 빠르게 달리기
    }
    else
    {
        Speed = MaxSpeed; // 일반 속도로 돌아가기
    }
}
