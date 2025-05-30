#include "AnimStateMachine.h"

#include "Lua/LuaScriptManager.h"

UAnimStateMachine::UAnimStateMachine()
{
    
}

void UAnimStateMachine::ProcessState()
{
    
}

void UAnimStateMachine::InitLuaStateMachine()
{
    LuaTable = FLuaScriptManager::Get().CreateLuaTable(LuaScriptName);

    FLuaScriptManager::Get().RegisterActiveAnimLua(this);
    if (!LuaTable.valid())
        return;

    
}



