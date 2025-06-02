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

    CameraComponent->FollowMainPlayer();
    std::shared_ptr<FEditorViewportClient> ViewportClient = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();
    float Width = ViewportClient->GetViewport()->GetD3DViewport().Width;
    float Height = ViewportClient->GetViewport()->GetD3DViewport().Height;
    GEngine->ActiveWorld->GetPlayerController()->SetLetterBoxWidthHeight(Width, Height);

    // 액터는 Serialize로직이 없어서 하드코딩
    ExplosionParticle = UAssetManager::Get().GetParticleSystem("Contents/ParticleSystem/UParticleSystem_368");
}

void ACharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    USkeletalMesh* SkeletalMeshAsset = MeshComponent->GetSkeletalMeshAsset();
    for (int i = 0; i < SkeletalMeshAsset->GetRenderData()->MaterialSubsets.Num(); i++)
    {
        FName MaterialName = SkeletalMeshAsset->GetRenderData()->MaterialSubsets[i].MaterialName;
        FVector EmissiveColor = FVector(0.0f, 0.0f, 0.0f);
        UAssetManager::Get().GetMaterial(MaterialName)->SetEmissive(EmissiveColor);
    }
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
    NewActor->ImpulseScale = ImpulseScale;
    NewActor->ExplosionParticle = ExplosionParticle;

    NewActor->DeathCameraTransitionTime = DeathCameraTransitionTime;
    NewActor->DeathLetterBoxTransitionTime = DeathLetterBoxTransitionTime;
    
    return NewActor;
}

void ACharacter::Tick(float DeltaTime)
{
    APawn::Tick(DeltaTime);

    DoCameraEffect(DeltaTime);
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
                    auto* RigidDynamic = Cast<UStaticMeshComponent>(Car->GetRootComponent())->BodyInstance->BIGameObject->DynamicRigidBody;
                    PxVec3 CurVelocity = RigidDynamic->getLinearVelocity();
                    CurVelocity *= 0.1f;
                    RigidDynamic->setLinearVelocity(CurVelocity);

                    UClass* CameraShakeClass = UDamageCameraShake::StaticClass();
                    GEngine->ActiveWorld->GetPlayerController()->ClientStartCameraShake(CameraShakeClass);
                    GEngine->ActiveWorld->GetPlayerController()->SetViewTarget(Car, Params);
                    GEngine->ActiveWorld->GetPlayerController()->Possess(Car);
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

void ACharacter::UpdateParticleEffectLocation()
{
    UParticleSystemComponent* PSC = GetComponentByClass<UParticleSystemComponent>();
    if (PSC)
    {

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
    if (HitComponent && 
        OtherComp && 
        MeshComponent && 
        HitComponent == CapsuleComponent && 
        OtherComp->GetOwner() &&
        OtherComp->GetOwner()->IsA<ACar>())
    {
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
        ParticleUtils::CreateParticleOnWorld(GetWorld(), ExplosionParticle, Hit.ImpactPoint, true, 0.2f);
    }

    if (HitComponent &&
        OtherComp &&
        MeshComponent &&
        HitComponent == CapsuleComponent &&
        OtherComp->GetOwner() &&
        OtherComp->GetOwner()->IsA<ARoad>())
    {
        ARoad* Road = Cast<ARoad>(OtherComp->GetOwner());
        Road->OnRed.AddLambda([this]()
        {
            USkeletalMesh* SkeletalMeshAsset = MeshComponent->GetSkeletalMeshAsset();
            for (int i = 0; i < SkeletalMeshAsset->GetRenderData()->MaterialSubsets.Num(); i++)
            {
                FName MaterialName = SkeletalMeshAsset->GetRenderData()->MaterialSubsets[i].MaterialName;
                FVector EmissiveColor = FVector(0.03f, 0.0f, 0.0f);
                UAssetManager::Get().GetMaterial(MaterialName)->SetEmissive(EmissiveColor);
            }
        });

        Road->OnNoRed.AddLambda([this]()
            {
                USkeletalMesh* SkeletalMeshAsset = MeshComponent->GetSkeletalMeshAsset();
                for (int i = 0; i < SkeletalMeshAsset->GetRenderData()->MaterialSubsets.Num(); i++)
                {
                    FName MaterialName = SkeletalMeshAsset->GetRenderData()->MaterialSubsets[i].MaterialName;
                    FVector EmissiveColor = FVector(0.0f, 0.0f, 0.0f);
                    UAssetManager::Get().GetMaterial(MaterialName)->SetEmissive(EmissiveColor);
                }
            });

        Road->OnDeath.AddLambda([this]()
        {
            bIsDead = true;
        });

        if (Road->GetCurrentRoadState() == ERoadState::Safe)
        {
            Road->SetIsOverlapped(true);
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

    }
}

float ACharacter::GetSpeed()
{
    PxVec3 CurrVelocity = CapsuleComponent->BodyInstance->BIGameObject->DynamicRigidBody->getLinearVelocity();
    if (bIsStop)
        CurrVelocity = PxVec3(0.0f, 0.0f, 0.0f);
    
    // UE_LOG(ELogLevel::Display, TEXT("Speed: %f"), CurrVelocity.magnitude());
    return CurrVelocity.magnitude();
}

void ACharacter::SetSpeed(float NewVelocity)
{
}

void ACharacter::MoveForward(float Value)
{
    bIsStop = false;
    
    if (bIsRunning)
        CurrentForce *= 2.0f;

    physx::PxRigidDynamic* PxCharActor = static_cast<physx::PxRigidDynamic*>(CapsuleComponent->BodyInstance->RigidActorSync);
    if (PxCharActor == nullptr)
        return;

    CurrentForce = FMath::Min(CurrentForce + ForceIncrement, MaxForce);
    FVector Forward = GetActorForwardVector().GetSafeNormal();
    float ForceScalar = CurrentForce * Value;
    physx::PxVec3 PushForce(
        Forward.X * ForceScalar,
        Forward.Y * ForceScalar,
        Forward.Z * ForceScalar
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
    bIsStop = false;
    
    if (bIsRunning)
        CurrentForce *= 2.0f;

    physx::PxRigidDynamic* PxCharActor = static_cast<physx::PxRigidDynamic*>(CapsuleComponent->BodyInstance->RigidActorSync);
    if (PxCharActor == nullptr)
        return;

    CurrentForce = FMath::Min(CurrentForce + ForceIncrement, MaxForce);
    FVector Right = GetActorRightVector().GetSafeNormal();
    float ForceScalar = CurrentForce * Value;
    
    physx::PxVec3 PushForce(
        Right.X * ForceScalar,
        Right.Y * ForceScalar,
        Right.Z * ForceScalar
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

void ACharacter::Stop()
{
    bIsStop = true;
    CurrentForce = 0.0f;

    physx::PxRigidDynamic* PxCharActor =
        static_cast<physx::PxRigidDynamic*>(CapsuleComponent->BodyInstance->RigidActorSync);
    if (PxCharActor == nullptr)
        return;

    PxCharActor->setLinearVelocity(physx::PxVec3(0.0f, 0.0f, 0.0f));
}
