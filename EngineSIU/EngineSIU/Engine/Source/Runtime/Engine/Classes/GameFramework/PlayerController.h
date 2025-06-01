#pragma once
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Actor.h" 
#include "Classes/Components/InputComponent.h"

class APlayerCameraManager;

class APlayerController : public AActor
{
    DECLARE_CLASS(APlayerController, AActor)
    
public:
    APlayerController() = default;
    virtual ~APlayerController() override = default;

    virtual void PostSpawnInitialize() override;

    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;
    
    void ProcessInput(float DeltaTime) const;

    virtual void Destroyed() override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UInputComponent* GetInputComponent() const { return InputComponent; }

    void SetViewTarget(class AActor* NewViewTarget, struct FViewTargetTransitionParams TransitionParams);

    virtual void Possess(AActor* InActor);

    virtual void UnPossess();
    
    virtual void BindAction(const FString& Key, const std::function<void(float)>& Callback);

    AActor* GetPossessedActor() const { return PossessedActor; }
    
    // 카메라 관련 함수
    AActor* GetViewTarget() const;

    virtual void SpawnPlayerCameraManager();

    void ClientStartCameraShake(UClass* Shake);

    void ClientStopCameraShake(UClass* Shake, bool bImmediately = true);

    void ClientStartCameraVignetteAnimation(float FromIntensity, float ToIntensity, float Duration);

    void ClientCameraVignetteColor(FLinearColor InColor);

    APlayerCameraManager* PlayerCameraManager = nullptr;

    void SetLetterBoxWidthHeight(float Width, float Height);
    void SetLetterBoxColor(const FLinearColor& Color);
    
protected:
    UPROPERTY(UInputComponent*, InputComponent, = nullptr)

    virtual void SetupInputComponent();

    AActor* PossessedActor = nullptr;

    bool bHasPossessed = false;
};

