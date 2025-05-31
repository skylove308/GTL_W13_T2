#include "AnimStateMachine.h"

#include "Lua/LuaScriptManager.h"

UAnimStateMachine::UAnimStateMachine()
{
    
}

void UAnimStateMachine::Initialize(APawn* InOwner, UAnimInstance* InAnimInstance)
{
    OwnerPawn = InOwner;
    OwningAnimInstance = InAnimInstance;
}

void UAnimStateMachine::ProcessState()
{
    if (!LuaTable.valid())
        return;

    sol::function UpdateFunc = LuaTable["Update"];
    if (!UpdateFunc.valid())
    {
        UE_LOG(ELogLevel::Warning, TEXT("Lua Update function not valid!"));
        return;
    }

    sol::object result = UpdateFunc(LuaTable, 0.0f);

    sol::table StateInfo = result.as<sol::table>();
    FString StateName = StateInfo["anim"].get_or(std::string("")).c_str();
    float Blend = StateInfo["blend"].get_or(0.f);


    UE_LOG(ELogLevel::Display, TEXT("Lua Test %s", *GetOuter()->GetName()));

    // if (!StateName.IsEmpty() && StateName != LastStateName)
    // {
    //     LastStateName = StateName;
    //
    //     // 애니메이션 이름을 AnimInstance에 전달
    //     if (OwnedAnimInstance)
    //     {
    //         UAnimSequence* Sequence = FResourceManager::LoadAnimationSequence(StateName);
    //         if (Sequence)
    //         {   
    //             OwnedAnimInstance->SetTargetSequence(Sequence, Blend);
    //         }
    //         else
    //         {
    //             UE_LOG(ELogLevel::Display, TEXT("AnimSequence not found for state: %s"), *StateName);
    //         }
    //     }
    // }
    
}

void UAnimStateMachine::InitLuaStateMachine()
{

    if (LuaScriptName.IsEmpty())
    {
        return;
    }
    LuaTable = FLuaScriptManager::Get().CreateLuaTable(LuaScriptName);

    FLuaScriptManager::Get().RegisterActiveAnimLua(this);
    if (!LuaTable.valid())
        return;
}



