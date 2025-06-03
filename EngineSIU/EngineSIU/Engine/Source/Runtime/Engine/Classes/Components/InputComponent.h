#pragma once

#include "Core/Container/Map.h"
#include "Delegates/DelegateCombination.h"
#include "Runtime/InputCore/InputCoreTypes.h"
#include "Components/ActorComponent.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOneFloatDelegate, const float&)
DECLARE_MULTICAST_DELEGATE(FOnKeyPressed);
DECLARE_MULTICAST_DELEGATE(FOnKeyReleased);

class UInputComponent : public UActorComponent
{
    DECLARE_CLASS(UInputComponent, UActorComponent)


public:
    UInputComponent() = default;
    virtual ~UInputComponent() override = default;
    void BindKeyPressAction(const FString& Key, const std::function<void(float)>& Callback);
    void BindOnKeyPressAction(const FString& Key, const std::function<void()>& Callback);
    void BindOnKeyReleasedAction(const FString& Key, const std::function<void()>& Callback);

    void ProcessInput(float DeltaTime);
    
    void SetPossess();
    void BindInputDelegate();
    void UnPossess();
    void ClearBindDelegate();
    // Possess가 풀렸다가 다시 왔을때 원래 바인딩 돼있던 애들 일괄적으로 다시 바인딩해줘야할수도 있음.
    void InputKey(const FKeyEvent& InKeyEvent);

private:
    TMap<FString, FOneFloatDelegate> KeyBindDelegate;
    TMap<FString, FOnKeyPressed> OnKeyPressedBindDelegate;
    TMap<FString, FOnKeyReleased> OnKeyReleasedBindDelegate;
    TArray<FDelegateHandle> BindKeyDownDelegateHandles;
    TArray<FDelegateHandle> BindKeyUpDelegateHandles;

    TSet<EKeys::Type> PressedKeys;
};
