#pragma once
#include<UObject/Object.h>
#include "UObject/ObjectMacros.h"

#include "sol/sol.hpp"

class UAnimInstance;
class APawn;

class UAnimStateMachine : public UObject
{
    DECLARE_CLASS(UAnimStateMachine, UObject)

public:
    UAnimStateMachine();
    virtual ~UAnimStateMachine() override = default;

    virtual void Initialize(APawn* InOwner, UAnimInstance* InAnimInstance);

    void ProcessState();
    
    void InitLuaStateMachine();
    
    FString GetLuaScriptName() const { return LuaScriptName; }

    APawn* OwnerPawn;
    UAnimInstance* OwningAnimInstance;
    
private:
    UPROPERTY(EditAnywhere, FString, LuaScriptName, = TEXT(""));
    sol::table LuaTable = {};

};
