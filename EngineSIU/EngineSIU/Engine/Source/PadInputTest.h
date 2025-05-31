#include "UserInterface/Console.h" // windows.h TCHAR 재정의 때문에 제일 앞에 정의해야 함

#include <windows.h>
#include <xinput.h>
#include <iostream>
#include <cmath>


// XInput 라이브러리 링크
#pragma comment(lib, "xinput.lib")

class XInputController
{
private:
    int PlayerIndex;
    XINPUT_STATE PreviousState;
    XINPUT_STATE CurrentState;
    
public:
    XInputController(int PlayerNum) : PlayerIndex(PlayerNum - 1)
    {
        ZeroMemory(&PreviousState, sizeof(XINPUT_STATE));
        ZeroMemory(&CurrentState, sizeof(XINPUT_STATE));
    }
    
    // 컨트롤러 연결 상태 확인
    bool IsConnected()
    {
        ZeroMemory(&CurrentState, sizeof(XINPUT_STATE));
        DWORD result = XInputGetState(PlayerIndex, &CurrentState);
        return (result == ERROR_SUCCESS);
    }
    
    // 컨트롤러 상태 업데이트
    void Update()
    {
        PreviousState = CurrentState;
        ZeroMemory(&CurrentState, sizeof(XINPUT_STATE));
        XInputGetState(PlayerIndex, &CurrentState);
    }
    
    // 버튼 입력 확인
    bool IsButtonPressed(WORD Button)
    {
        return (CurrentState.Gamepad.wButtons & Button) != 0;
    }
    
    // 버튼이 방금 눌렸는지 확인 (이전 프레임에는 안 눌렸고 현재 프레임에 눌림)
    bool IsButtonJustPressed(WORD Button)
    {
        return (CurrentState.Gamepad.wButtons & Button) != 0 &&
               (PreviousState.Gamepad.wButtons & Button) == 0;
    }
    
        // 아날로그 스틱 값 가져오기 (-1.0f ~ 1.0f 범위로 정규화)
        float GetLeftStickX()
    {
        SHORT rawValue = CurrentState.Gamepad.sThumbLX;
        
        if (abs(rawValue) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            return 0.0f;
            
        if (rawValue > 0)
            return static_cast<float>(rawValue - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / 
                   static_cast<float>(32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        else
            return static_cast<float>(rawValue + XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / 
                   static_cast<float>(32768 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    }

    float GetLeftStickY()
    {
        SHORT rawValue = CurrentState.Gamepad.sThumbLY;
        
        if (abs(rawValue) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
            return 0.0f;
            
        if (rawValue > 0)
            return static_cast<float>(rawValue - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / 
                   static_cast<float>(32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        else
            return static_cast<float>(rawValue + XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / 
                   static_cast<float>(32768 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
    }

    float GetRightStickX()
    {
        SHORT rawValue = CurrentState.Gamepad.sThumbRX;
        
        if (abs(rawValue) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
            return 0.0f;
            
        if (rawValue > 0)
            return static_cast<float>(rawValue - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) / 
                   static_cast<float>(32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        else
            return static_cast<float>(rawValue + XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) / 
                   static_cast<float>(32768 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    }

    float GetRightStickY()
    {
        SHORT rawValue = CurrentState.Gamepad.sThumbRY;
        
        if (abs(rawValue) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
            return 0.0f;
            
        if (rawValue > 0)
            return static_cast<float>(rawValue - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) / 
                   static_cast<float>(32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        else
            return static_cast<float>(rawValue + XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) / 
                   static_cast<float>(32768 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
    }
    
    // 트리거 값 가져오기 (0.0f ~ 1.0f)
    float GetLeftTrigger()
    {
        BYTE trigger = CurrentState.Gamepad.bLeftTrigger;
        if (trigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            return 0.0f;
        return (trigger - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / (255.0f - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
    }
    
    float GetRightTrigger()
    {
        BYTE trigger = CurrentState.Gamepad.bRightTrigger;
        if (trigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            return 0.0f;
        return (trigger - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / (255.0f - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
    }
    
    // 진동 설정 (0.0f ~ 1.0f)
    void SetVibration(float leftMotor, float rightMotor)
    {
        XINPUT_VIBRATION vibration;
        ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
        
        vibration.wLeftMotorSpeed = (WORD)(leftMotor * 65535);
        vibration.wRightMotorSpeed = (WORD)(rightMotor * 65535);
        
        XInputSetState(PlayerIndex, &vibration);
    }
};

// // DirectX 메인 루프에서 사용할 예제
// class Game
// {
// private:
//     XInputController Controller;
//     
// public:
//     Game(int PlayerIndex) : Controller(PlayerIndex)
//     {
//     }
//     
//     void Update()
//     {
//         // 컨트롤러 상태 업데이트
//         Controller.Update();
//         
//         if (!Controller.IsConnected())
//         {
//             std::cout << "Controller not connected!" << std::endl;
//             return;
//         }
//         
//         // 버튼 입력 처리
//         if (Controller.IsButtonJustPressed(XINPUT_GAMEPAD_A))
//         {
//             UE_LOG(ELogLevel::Display, "Press A Button");
//             std::cout << "A button pressed!" << std::endl;
//             // 진동 효과 (0.5초간 50% 강도)
//             Controller.SetVibration(0.5f, 0.5f);
//         }
//         
//         if (Controller.IsButtonPressed(XINPUT_GAMEPAD_B))
//         {
//             std::cout << "B button held!" << std::endl;
//         }
//         
//         // 아날로그 스틱 입력 처리
//         float leftX = Controller.GetLeftStickX();
//         float leftY = Controller.GetLeftStickY();
//         
//         if (abs(leftX) > 0.1f || abs(leftY) > 0.1f)
//         {
//             std::cout << "Left stick: X=" << leftX << ", Y=" << leftY << std::endl;
//         }
//         
//         // 트리거 입력 처리
//         float leftTrigger = Controller.GetLeftTrigger();
//         float rightTrigger = Controller.GetRightTrigger();
//         
//         if (leftTrigger > 0.1f)
//         {
//             std::cout << "Left trigger: " << leftTrigger << std::endl;
//         }
//         
//         if (rightTrigger > 0.1f)
//         {
//             std::cout << "Right trigger: " << rightTrigger << std::endl;
//         }
//         
//         // 방향패드 처리
//         if (Controller.IsButtonPressed(XINPUT_GAMEPAD_DPAD_UP))
//         {
//             std::cout << "D-Pad Up pressed!" << std::endl;
//         }
//     }
//     
//     void Shutdown()
//     {
//         // 진동 정지
//         Controller.SetVibration(0.0f, 0.0f);
//     }
// };
