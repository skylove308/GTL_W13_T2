#pragma once
#include<UObject/Object.h>
#include "UObject/ObjectMacros.h"

#include "sol/sol.hpp"

class APawn;

class UAnimStateMachine : public UObject
{
    DECLARE_CLASS(UAnimStateMachine, UObject)

public:
    UAnimStateMachine();
    virtual ~UAnimStateMachine() override = default;

    void ProcessState();
    
    void InitLuaStateMachine();
    
    FString GetLuaScriptName() const { return LuaScriptName; }

private:
    UPROPERTY(EditAnywhere, FString, LuaScriptName, = TEXT(""));
    sol::table LuaTable;

};
