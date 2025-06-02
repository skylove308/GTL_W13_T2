#include "Map.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"
#include "Lua/LuaScriptComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"

AMap::AMap()
{
    UStaticMeshComponent* StaticMeshComp = AddComponent<UStaticMeshComponent>("StreetMap");
    StaticMeshComp->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Primitives/CubePrimitive.Obj"));
    RootComponent = StaticMeshComp;
}

void AMap::BeginPlay()
{
    Super::BeginPlay();
}

UObject* AMap::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    return NewActor;
}


void AMap::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(AMap, sol::bases<AActor>(),

    )
}


bool AMap::BindSelfLuaProperties()
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
