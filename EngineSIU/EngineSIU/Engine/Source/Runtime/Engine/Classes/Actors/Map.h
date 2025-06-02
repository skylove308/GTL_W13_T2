#pragma once
#include "GameFramework/Actor.h"

enum class EMapObjectType : uint8
{
    None,
    Car,
    Train
};

class AMap : public AActor
{
    DECLARE_CLASS(AMap, AActor)
public:
    AMap();
    virtual ~AMap() override = default;
    virtual void BeginPlay() override;
    virtual UObject* Duplicate(UObject* InOuter) override;

public:
    virtual void RegisterLuaType(sol::state& Lua) override;
    virtual bool BindSelfLuaProperties() override;

    void SpawnObject();
};

