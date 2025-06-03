#include "GameUIManager.h"

#include "GameManager.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"
#include "Lua/LuaScriptComponent.h"

AGameUIManager::AGameUIManager()
{
}

void AGameUIManager::BeginPlay()
{
    Super::BeginPlay();
    LuaScriptComponent->SetScriptName(TEXT("GameUIManager"));
}

void AGameUIManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //int CurrScore = AGameManager::Instance().GetScore();
    //if (CurrentScore != CurrScore)
    //    CurrentScore = CurrScore;
}

void AGameUIManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

UObject* AGameUIManager::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewActor->CurrentScore = CurrentScore;
    
    return NewActor;
}

void AGameUIManager::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(AGameUIManager, sol::bases<AActor>(),
        "Score", sol::property(&ThisClass::GetScore, &ThisClass::SetScore)
        // "InitialAngularVelocity", sol::property(&ThisClass::GetInitialAngularVelocity, &ThisClass::SetInitialAngularVelocity),
        // "Mass", sol::property(&ThisClass::GetMass, &ThisClass::SetMass),
        // "LinearDamping", sol::property(&ThisClass::GetLinearDamping, &ThisClass::SetLinearDamping),
        // "AngularDamping", sol::property(&ThisClass::GetAngularDamping, &ThisClass::SetAngularDamping),
        // "Drive", &ThisClass::Drive
    )
}

bool AGameUIManager::BindSelfLuaProperties()
{
    if (!Super::BindSelfLuaProperties())
        return false;

    sol::table& LuaTable = LuaScriptComponent->GetLuaSelfTable();
    if (!LuaTable.valid())
    {
        return false;
    }

    LuaTable["this"] = this;
    LuaTable["Name"] = *GetName();

    return true;
}
