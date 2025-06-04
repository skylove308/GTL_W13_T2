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
#include "Engine/Contents/Objects/DamageCameraShake.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealClient.h"
#include "Components/StaticMeshComponent.h"
#include "Actors/Road.h"
#include <Particles/ParticleSystemComponent.h>
#include "ParticleHelper.h"
#include "Engine/SkeletalMesh.h"
#include "SoundManager.h"
#include "Actors/GameManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "Actors/Camera.h"
#include "Actors/StreetLight.h"
#include "Engine/Contents/Maps/MapModule.h"

class AStreetLight;

ACharacter::ACharacter()
{
    // SetActorLocation(FVector(0.0f, ))
    
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

    CameraBoom = AddComponent<USpringArmComponent>("CameraBoom");
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.0f; // 카메라와 캐릭터 사이의 거리

    CameraComponent = AddComponent<UCameraComponent>("FollowCamera");
    CameraComponent->SetupAttachment(CameraBoom);

    FSoundManager::GetInstance().LoadSound("CarCrash", "Contents/Sounds/CarCrash.wav");
    FSoundManager::GetInstance().LoadSound("Wasted", "Contents/Sounds/Wasted.wav");
    FSoundManager::GetInstance().LoadSound("Title", "Contents/Sounds/Title.mp3");
}

void ACharacter::BeginPlay()
{
    APawn::BeginPlay();

    CameraComponent->FollowMainPlayer();
    std::shared_ptr<FEditorViewportClient> ViewportClient = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
    float Width = ViewportClient->GetViewport()->GetD3DViewport().Width;
    float Height = ViewportClient->GetViewport()->GetD3DViewport().Height;
    GEngine->ActiveWorld->GetPlayerController()->SetLetterBoxWidthHeight(Width, Height);

    auto Actors = GEngine->ActiveWorld->GetActiveLevel()->Actors;
    for (auto Actor : Actors)
    {
        if (Actor->IsA<AGameManager>())
        {
            GameManager = Cast<AGameManager>(Actor);
            break;
        }
    }

    // 액터는 Serialize로직이 없어서 하드코딩
    ExplosionParticle = UAssetManager::Get().GetParticleSystem("Contents/ParticleSystem/UParticleSystem_368");
    FSoundManager::GetInstance().PlaySound("Title");
    //CapsuleComponent->BodyInstance->BIGameObject->DynamicRigidBody->setMass(10.0f);

    BindInput();
}

void ACharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    
    USkeletalMesh* SkeletalMeshAsset = MeshComponent->GetSkeletalMeshAsset();
    for (int i = 0; i < SkeletalMeshAsset->GetRenderData()->MaterialSubsets.Num(); i++)
    {
        FName MaterialName = SkeletalMeshAsset->GetRenderData()->MaterialSubsets[i].MaterialName;
        FVector EmissiveColor = FVector(0.0f, 0.0f, 0.0f);
        UAssetManager::Get().GetMaterial(MaterialName)->SetEmissive(EmissiveColor);
    }
    UnbindInput();
    Super::EndPlay(EndPlayReason);
}

UObject* ACharacter::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    if (!NewActor) return nullptr;

    NewActor->CapsuleComponent = NewActor->GetComponentByClass<UCapsuleComponent>();
    NewActor->MeshComponent = NewActor->GetComponentByClass<USkeletalMeshComponent>();
    NewActor->MovementComponent = NewActor->GetComponentByClass<UCharacterMovementComponent>();
    NewActor->CameraComponent = NewActor->GetComponentByClass<UCameraComponent>();
    NewActor->CameraBoom = NewActor->GetComponentByClass<USpringArmComponent>();

    if (NewActor->MovementComponent && NewActor->CapsuleComponent)
    {
        NewActor->MovementComponent->UpdatedComponent = NewActor->CapsuleComponent;
    }
    NewActor->ImpulseScale = ImpulseScale;
    NewActor->ExplosionParticle = ExplosionParticle;

    NewActor->DeathCameraTransitionTime = DeathCameraTransitionTime;
    NewActor->DeathLetterBoxTransitionTime = DeathLetterBoxTransitionTime;
    return NewActor;
}

void ACharacter::Tick(float DeltaTime)
{
    APawn::Tick(DeltaTime);

    Move(DeltaTime);
    //Rotate(DeltaTime);
    DoCameraEffect(DeltaTime);
    if (bOnSlomo)
    {
        assert(UEngine::TimeScale > KINDA_SMALL_NUMBER);
        SlomoTime += DeltaTime * 1.f / UEngine::TimeScale;
        if (SlomoTime >= SlomoDuration)
        {
            bOnSlomo = false;
            UEngine::TimeScale = 1.0f; // 슬로우모션 종료
            SlomoTime = 0.0f;
        }
    }
}

void ACharacter::DoCameraEffect(float DeltaTime)
{
    if (!bCameraEffect) return;

    if (CurrentDeathCameraTransitionTime <= 0.0f)
    {
        if (CurrentDeathLetterBoxTransitionTime > 0.0f)
        {
            std::shared_ptr<FEditorViewportClient> ViewportClient = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
            float Width = ViewportClient->GetViewport()->GetD3DViewport().Width;
            float Height = ViewportClient->GetViewport()->GetD3DViewport().Height;

            float LetterBoxWidth = Width;
            float LetterBoxHeight = (Height - Width * 0.5f) * (CurrentDeathLetterBoxTransitionTime / DeathLetterBoxTransitionTime) + (Width * 0.5f);

            GEngine->ActiveWorld->GetPlayerController()->SetLetterBoxWidthHeight(LetterBoxWidth, LetterBoxHeight);
            GEngine->ActiveWorld->GetPlayerController()->ClientCameraVignetteColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));
            GEngine->ActiveWorld->GetPlayerController()->ClientStartCameraVignetteAnimation(1.0f, 0.5f, 0.5f);
            GEngine->ActiveWorld->GetPlayerController()->SetLetterBoxColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f));

            CurrentDeathLetterBoxTransitionTime -= DeltaTime;
        }
        else
        {
            CurrentDeathCameraTransitionTime = DeathCameraTransitionTime; // 카메라 전환 시간 초기화
            bCameraEffect = false; // 카메라 효과 종료
            GameManager->SetState(EGameState::GameOver);
        }
    }

    if (!bSwitchCamera)
    {
        for (auto Actor : GEngine->ActiveWorld->GetActiveLevel()->Actors)
        {
            if (ACar* Car = Cast<ACar>(Actor))
            {
                FViewTargetTransitionParams Params;
                Params.BlendTime = DeathCameraTransitionTime; // 카메라 전환 시간
                {
                    //auto* RigidDynamic = Cast<UStaticMeshComponent>(Car->GetRootComponent())->BodyInstance->BIGameObject->DynamicRigidBody;
                    //PxVec3 CurVelocity = RigidDynamic->getLinearVelocity();
                    //CurVelocity *= 0.1f;
                    //RigidDynamic->setLinearVelocity(CurVelocity);

                    UClass* CameraShakeClass = UDamageCameraShake::StaticClass();
                    GEngine->ActiveWorld->GetPlayerController()->ClientStartCameraShake(CameraShakeClass);

                    ACamera* DeathCam = GEngine->ActiveWorld->SpawnActor<ACamera>();
                    DeathCam->SetActorLocation(GetActorLocation() + FVector(0.0f, 0.0f, 1000.0f));
                    if (Car->GetSpawnDirectionRight())
                    {
                        DeathCam->SetActorRotation(FRotator(-30.0f, -90.0f, 0.0f));
                    }
                    else
                    {
                        DeathCam->SetActorRotation(FRotator(-30.0f, 90.0f, 0.0f));
                    }

                    GEngine->ActiveWorld->GetPlayerController()->SetViewTarget(DeathCam, Params);
                    GEngine->ActiveWorld->GetPlayerController()->Possess(DeathCam);
                }
                bSwitchCamera = true;
                break;
            }
        }
    }

    if (bSwitchCamera && CurrentDeathCameraTransitionTime > 0.0f)
    {
        CurrentDeathCameraTransitionTime -= DeltaTime;
    }

}

void ACharacter::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(ACharacter, sol::bases<AActor>(),
        "Velocity", sol::property(&ThisClass::GetSpeed, &ThisClass::SetSpeed),
        "IsRunning", sol::property(&ThisClass::GetIsRunning, &ThisClass::SetIsRunning),
        "IsDead", sol::property(&ThisClass::GetIsDead, &ThisClass::SetIsDead)
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

void ACharacter::OnCollisionEnter(UPrimitiveComponent* HitComponent, UPrimitiveComponent* OtherComp, const FHitResult& Hit)
{
    if (!bIsDead &&
        HitComponent && 
        OtherComp && 
        MeshComponent && 
        HitComponent == CapsuleComponent && 
        OtherComp->GetOwner() &&
        OtherComp->GetOwner()->IsA<ACar>())
    {
        bIsDead = true;
        MeshComponent->RigidBodyType = ERigidBodyType::DYNAMIC; // 충돌 시 동적 물리로 변경
        MeshComponent->OnChangeRigidBodyFlag();

        //FVector ImpulseDirection = Hit.ImpactNormal + FVector(0.0f, 0.0f, 1.0f); // 충돌 방향 + 위쪽 방향
        //ImpulseDirection.Normalize(); // 방향 벡터 정규화
        float i = ImpulseScale;
        MeshComponent->AddImpulseToBones(Hit.ImpactNormal, ImpulseScale); // 임펄스 추가 (힘을 주는 효과)

        // !TODO : 차랑 부딪혔을 때 추가적인 로직 구현
        // 힘을 준다던지, 캡슐을 비활성화하고 SkeletalMeshComp를 루트컴포넌트로 한다던지, 등등..
        bCameraEffect = true;

        //ParticleUtils::CreateParticleOnWorld(GetWorld(), ExplosionParticle, Hit.ImpactPoint);
        //ParticleUtils::CreateParticleOnWorld(GetWorld(), ExplosionParticle, Hit.ImpactPoint, true, 1.f);

        FSoundManager::GetInstance().PlaySound("CarCrash");
        FSoundManager::GetInstance().PlaySound("Wasted", 1000);
        UEngine::TimeScale = 0.1f;
        bOnSlomo = true;
        SlomoTime = 0.0f;
    }

    if (HitComponent &&
        OtherComp &&
        MeshComponent &&
        HitComponent == CapsuleComponent &&
        OtherComp->GetOwner() &&
        OtherComp->GetOwner()->IsA<ARoad>())
    {
        CurrentRoad = Cast<ARoad>(OtherComp->GetOwner());
        CurrentRoad->OnRed.AddLambda([this]()
        {
            USkeletalMesh* SkeletalMeshAsset = MeshComponent->GetSkeletalMeshAsset();
            for (int i = 0; i < SkeletalMeshAsset->GetRenderData()->MaterialSubsets.Num(); i++)
            {
                FName MaterialName = SkeletalMeshAsset->GetRenderData()->MaterialSubsets[i].MaterialName;
                FVector EmissiveColor = FVector(0.03f, 0.0f, 0.0f);
                UAssetManager::Get().GetMaterial(MaterialName)->SetEmissive(EmissiveColor);
            }
        });

        CurrentRoad->OnNoRed.AddLambda([this]()
            {
                USkeletalMesh* SkeletalMeshAsset = MeshComponent->GetSkeletalMeshAsset();
                for (int i = 0; i < SkeletalMeshAsset->GetRenderData()->MaterialSubsets.Num(); i++)
                {
                    FName MaterialName = SkeletalMeshAsset->GetRenderData()->MaterialSubsets[i].MaterialName;
                    FVector EmissiveColor = FVector(0.0f, 0.0f, 0.0f);
                    UAssetManager::Get().GetMaterial(MaterialName)->SetEmissive(EmissiveColor);
                }
            });

        CurrentRoad->OnDeath.AddLambda([this]()
        {
            bIsDead = true;
            GameManager->SetState(EGameState::GameOver);
        });

        if (CurrentRoad->GetCurrentRoadState() == ERoadState::Safe)
            
        {
            CurrentRoad->SetIsOverlapped(true);
            CurrentRoad->TurnOnStreetLights();
        }
    }
}

void ACharacter::OnCollisionExit(UPrimitiveComponent* HitComponent, UPrimitiveComponent* OtherComp)
{
    if (HitComponent && 
        OtherComp && 
        MeshComponent && 
        HitComponent == CapsuleComponent && 
        OtherComp->GetOwner() &&
        OtherComp->GetOwner()->IsA<ARoad>())
    {
        ARoad* Road = Cast<ARoad>(OtherComp->GetOwner());
        Road->SetIsOverlapped(false);

        // if (Road->GetCurrentRoadState() == ERoadState::Safe)
        // {
        //     Road->DestroyStreetLights();
        // }
    }
}

float ACharacter::GetSpeed()
{
    PxVec3 CurrVelocity = CapsuleComponent->BodyInstance->BIGameObject->DynamicRigidBody->getLinearVelocity();
    if (bIsStop)
        CurrVelocity = PxVec3(0.0f, 0.0f, 0.0f);
    
    return CurrVelocity.magnitude();
}

void ACharacter::SetSpeed(float NewVelocity)
{
}

void ACharacter::BindInput()
{
    UWorld* ActiveWorld = GetWorld();
    if (!ActiveWorld)
        return;

    ActiveWorld->GetPlayerController()->BindKeyPressAction("Run",
        [&](float Value) {
            bIsRunning = true;
        }
    );
    ActiveWorld->GetPlayerController()->BindKeyPressAction("RunRelease",
        [&](float Value) {
            bIsRunning = false;
        }
    );
    ActiveWorld->GetPlayerController()->BindKeyPressAction("Idle",
        [&](float Value) {
            Stop();
        });

    ActiveWorld->GetPlayerController()->BindOnKeyPressAction("W",
        [&]() {
            MoveInput.Y = 1.0f;
        });

    ActiveWorld->GetPlayerController()->BindOnKeyReleaseAction("W",
        [&]() {
            MoveInput.Y = 0.0f;
        });

    ActiveWorld->GetPlayerController()->BindOnKeyPressAction("S",
        [&]() {
            MoveInput.Y = -1.0f;
        });

    ActiveWorld->GetPlayerController()->BindOnKeyReleaseAction("S",
        [&]() {
            MoveInput.Y = 0.0f;
        });

    ActiveWorld->GetPlayerController()->BindOnKeyPressAction("A",
        [&]() {
            MoveInput.X = -1.0f;
        });

    ActiveWorld->GetPlayerController()->BindOnKeyReleaseAction("A",
        [&]() {
            MoveInput.X = 0.0f;
        });

    ActiveWorld->GetPlayerController()->BindOnKeyPressAction("D",
        [&]() {
            MoveInput.X = 1.0f;
        });

    ActiveWorld->GetPlayerController()->BindOnKeyReleaseAction("D",
        [&]() {
            MoveInput.X = 0.0f;
        });
}

void ACharacter::UnbindInput()
{

}

void ACharacter::Move(float DeltaTime)
{
    if (MoveInput.IsNearlyZero() || !CameraBoom)
        return;

    // 1. 카메라 기준 방향 구하기
    FVector CameraForward = CameraBoom->GetCameraForwardVector();
    FVector CameraRight = CameraBoom->GetCameraRightVector();

    // 수평면으로 투영 (Pitch 제거)
    CameraForward.Z = 0;
    CameraRight.Z = 0;
    CameraForward.Normalize();
    CameraRight.Normalize();

    // 2. 입력값을 카메라 방향 기준으로 변환
    FVector MoveDirection = CameraForward * MoveInput.Y + CameraRight * MoveInput.X;

    if (!MoveDirection.IsNearlyZero())
    {
        MoveDirection.Normalize();

        // 3. 이동 적용
        ApplyMovementForce(MoveDirection, 1.0f);

        // 4. 회전 적용 (부드럽게)
        FRotator TargetRotation = MoveDirection.Rotation();
        FRotator CurrentRotation = CapsuleComponent->GetRelativeRotation();

        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, MeshRotationSpeed);
        CapsuleComponent->SetRelativeRotation(NewRotation);
        UpdatePhysXTransform(GetActorLocation(), NewRotation.Quaternion());
    }
}


void ACharacter::ApplyMovementForce(const FVector& Direction, float Scale)
{
    physx::PxRigidDynamic* PxCharActor =
        static_cast<physx::PxRigidDynamic*>(CapsuleComponent->BodyInstance->RigidActorSync);
    if (PxCharActor == nullptr)
        return;

    if (bIsStop)
    {
        // 현재 속도를 0으로 초기화
        physx::PxRigidDynamic* PxCharActor = static_cast<physx::PxRigidDynamic*>(CapsuleComponent->BodyInstance->RigidActorSync);
        if (PxCharActor)
        {
            PxCharActor->setLinearDamping(InputLinearDamping);
        }
        bIsStop = false;
    }

    float CurrentMaxSpeed = bIsRunning ? RunMaxSpeed : WalkMaxSpeed;
    float CurrentForce = bIsRunning ? RunForce : WalkForce;

    float Speed = GetSpeed();

    if (Speed < CurrentMaxSpeed)
    {
        PxVec3 PushForce(
            Direction.X * CurrentForce * Scale,
            Direction.Y * CurrentForce * Scale,
            Direction.Z * CurrentForce * Scale
        );

        PxCharActor->addForce(PushForce, physx::PxForceMode::eFORCE, /*autowake=*/ true);
    }
}

void ACharacter::Stop()
{
    if (GameManager && GameManager->GetState() != EGameState::Playing)
        return;
    
    bIsStop = true;

    physx::PxRigidDynamic* PxCharActor =
        static_cast<physx::PxRigidDynamic*>(CapsuleComponent->BodyInstance->RigidActorSync);
    if (PxCharActor == nullptr)
        return;

    PxCharActor->setLinearDamping(NoInputLinearDamping);
}

void ACharacter::Rotate(float DeltaTime)
{
    PxRigidDynamic* PxCharActor =
        static_cast<physx::PxRigidDynamic*>(CapsuleComponent->BodyInstance->RigidActorSync);

    if (PxCharActor == nullptr)
        return;

    PxVec3 CurrVelocity = PxCharActor->getLinearVelocity();
    FVector Velocity = FVector(CurrVelocity.x, CurrVelocity.y, 0);
    if (Velocity.SizeSquared() > KINDA_SMALL_NUMBER && !bIsStop)
    {
        FRotator TargetRotation = CameraBoom ? CameraBoom->GetCameraForwardVector().Rotation() : Velocity.Rotation();// FVector → FRotator

        FRotator CurrentRotation = GetActorRotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, MeshRotationSpeed);
        FQuat NewRotQuat = NewRotation.Quaternion();

        SetActorRotation(TargetRotation);
        UpdatePhysXTransform(GetActorLocation(), NewRotQuat);

        UE_LOG(ELogLevel::Warning, "Character Rotation %f, %f, %f", CurrentRotation.Pitch, CurrentRotation.Yaw, CurrentRotation.Roll);
        UE_LOG(ELogLevel::Warning, "TargetRotation Rotation %f, %f, %f", TargetRotation.Pitch, TargetRotation.Yaw, TargetRotation.Roll);
    }
}

void ACharacter::UpdatePhysXTransform(const FVector& Location, const FQuat& Rotation)
{
    PxRigidDynamic* PxCharActor =
        static_cast<physx::PxRigidDynamic*>(CapsuleComponent->BodyInstance->RigidActorSync);

    if (PxCharActor == nullptr)
        return;

    FVector CurLocation = GetActorLocation();
    FQuat CurRotation = GetActorRotation().Quaternion();

    PxTransform TargetTransform = PxTransform(
        PxVec3(Location.X, Location.Y, Location.Z),
        PxQuat(Rotation.X, Rotation.Y, Rotation.Z, Rotation.W)
    );

    PxCharActor->setGlobalPose(TargetTransform);
}
