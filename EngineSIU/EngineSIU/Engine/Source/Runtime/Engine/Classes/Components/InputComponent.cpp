#include "InputComponent.h"

void UInputComponent::ProcessInput(float DeltaTime)
{
    if (PressedKeys.Contains(EKeys::W))
    {
        KeyBindDelegate[FString("W")].Broadcast(DeltaTime);
    }
    if (PressedKeys.Contains(EKeys::A))
    {
        KeyBindDelegate[FString("A")].Broadcast(DeltaTime);
    }
    if (PressedKeys.Contains(EKeys::S))
    {
        KeyBindDelegate[FString("S")].Broadcast(DeltaTime);
    }
    if (PressedKeys.Contains(EKeys::D))
    {
        KeyBindDelegate[FString("D")].Broadcast(DeltaTime);
    }
    if (PressedKeys.Contains(EKeys::SpaceBar))
    {
        KeyBindDelegate[FString("Run")].Broadcast(DeltaTime);
    }
    if (!PressedKeys.Contains(EKeys::SpaceBar))
    {
        KeyBindDelegate[FString("RunRelease")].Broadcast(DeltaTime);
    }
    if (!PressedKeys.Contains(EKeys::W) && !PressedKeys.Contains(EKeys::A) 
        && !PressedKeys.Contains(EKeys::S) && !PressedKeys.Contains(EKeys::D))
    {
        KeyBindDelegate[FString("Idle")].Broadcast(DeltaTime);
    }
}

void UInputComponent::SetPossess()
{
    BindInputDelegate();
    
    //TODO: Possess일때 기존에 있던거 다시 넣어줘야할수도
}

void UInputComponent::BindInputDelegate()
{
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    BindKeyDownDelegateHandles.Add(Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
    {
        InputKey(InKeyEvent);
    }));

    BindKeyUpDelegateHandles.Add(Handler->OnKeyUpDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
    {
        InputKey(InKeyEvent);
    }));
    
}

void UInputComponent::UnPossess()
{ 
    ClearBindDelegate();
}

void UInputComponent::ClearBindDelegate()
{
    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();

    for (FDelegateHandle DelegateHandle : BindKeyDownDelegateHandles)
    {
        Handler->OnKeyDownDelegate.Remove(DelegateHandle);
    }
     
    for (FDelegateHandle DelegateHandle : BindKeyUpDelegateHandles)
    {
        Handler->OnKeyUpDelegate.Remove(DelegateHandle);
    }
    
    BindKeyDownDelegateHandles.Empty();
    BindKeyUpDelegateHandles.Empty();
}

void UInputComponent::InputKey(const FKeyEvent& InKeyEvent)
{
    // 일반적인 단일 키 이벤트
    switch (InKeyEvent.GetCharacter())
    {
    case 'W':
        {
            if (InKeyEvent.GetInputEvent() == IE_Pressed)
            {
                PressedKeys.Add(EKeys::W);
                OnKeyPressedBindDelegate[FString("W")].Broadcast();
            }
            else if (InKeyEvent.GetInputEvent() == IE_Released)
            {
                PressedKeys.Remove(EKeys::W);
                OnKeyReleasedBindDelegate[FString("W")].Broadcast();
            }
            break;
        }
    case 'A':
        {
            if (InKeyEvent.GetInputEvent() == IE_Pressed)
            {
                PressedKeys.Add(EKeys::A);
                OnKeyPressedBindDelegate[FString("A")].Broadcast();
            }
            else if (InKeyEvent.GetInputEvent() == IE_Released)
            {
                PressedKeys.Remove(EKeys::A);
                OnKeyReleasedBindDelegate[FString("A")].Broadcast();
            }
            break;
        }
    case 'S':
        {
            if (InKeyEvent.GetInputEvent() == IE_Pressed)
            {
                PressedKeys.Add(EKeys::S);
                OnKeyPressedBindDelegate[FString("S")].Broadcast();
            }
            else if (InKeyEvent.GetInputEvent() == IE_Released)
            {
                PressedKeys.Remove(EKeys::S);
                OnKeyReleasedBindDelegate[FString("S")].Broadcast();
            }
            break;
        }
    case 'D':
        {
            if (InKeyEvent.GetInputEvent() == IE_Pressed)
            {
                PressedKeys.Add(EKeys::D);
                OnKeyPressedBindDelegate[FString("D")].Broadcast();
            }
            else if (InKeyEvent.GetInputEvent() == IE_Released)
            {
                PressedKeys.Remove(EKeys::D);
                OnKeyReleasedBindDelegate[FString("D")].Broadcast();
            }
            break;
        }
    case 32: // SpaceBar
        {
            if (InKeyEvent.GetInputEvent() == IE_Pressed)
            {
                PressedKeys.Add(EKeys::SpaceBar);
                OnKeyPressedBindDelegate[FString("Run")].Broadcast();
            }
            else if (InKeyEvent.GetInputEvent() == IE_Released)
            {
                PressedKeys.Remove(EKeys::SpaceBar);
                OnKeyReleasedBindDelegate[FString("RunRelease")].Broadcast();
            }
            break;
        }
    default:
        break;
    }
}

void UInputComponent::BindKeyPressAction(const FString& Key, const std::function<void(float)>& Callback)
{
    if (Callback == nullptr)
    {
        return;
    }
    
    KeyBindDelegate[Key].AddLambda([this, Callback](float DeltaTime)
    {
        Callback(DeltaTime);
    });
}

void UInputComponent::BindOnKeyPressAction(const FString& Key, const std::function<void()>& Callback)
{
    if (Callback == nullptr)
    {
        return;
    }

    OnKeyPressedBindDelegate[Key].AddLambda([this, Callback]()
        {
            Callback();
        });
}

void UInputComponent::BindOnKeyReleasedAction(const FString& Key, const std::function<void()>& Callback)
{
    if (Callback == nullptr)
    {
        return;
    }

    OnKeyReleasedBindDelegate[Key].AddLambda([this, Callback]()
        {
            Callback();
        });
}
