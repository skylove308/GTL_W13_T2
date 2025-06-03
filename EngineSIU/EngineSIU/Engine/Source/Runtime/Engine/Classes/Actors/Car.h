#pragma once
#include "GameFramework/Actor.h"

class ACar : public AActor
{
    DECLARE_CLASS(ACar, AActor)
public:
    ACar();
    virtual void BeginPlay() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void PostSpawnInitialize() override;

public:
    virtual void RegisterLuaType(sol::state& Lua) override; // Lua에 클래스 등록해주는 함수.
    virtual bool BindSelfLuaProperties() override; // LuaEnv에서 사용할 멤버 변수 등록 함수.
     
    void Drive();

private:
    UPROPERTY_WITH_FLAGS(VisibleAnywhere, FVector, InitialVelocity, = FVector::ZeroVector)
    UPROPERTY_WITH_FLAGS(VisibleAnywhere, FVector, InitialAngularVelocity, = FVector::ZeroVector)
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, Mass, = 1000.f) // 킬로그램 단위
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, LinearDamping, = 0.1f) // 선형 감쇠
    UPROPERTY_WITH_FLAGS(EditAnywhere, float, AngularDamping, = 0.1f) // 각 감쇠

public:
    //getter
    FVector GetInitialVelocity() const { return InitialVelocity; }
    FVector GetInitialAngularVelocity() const { return InitialAngularVelocity; }
    float GetMass() const { return Mass; }
    float GetLinearDamping() const { return LinearDamping; }
    float GetAngularDamping() const { return AngularDamping; }

    //setter
    void SetInitialVelocity(const FVector& InVelocity) { InitialVelocity = InVelocity; }
    void SetInitialAngularVelocity(const FVector& InAngularVelocity) { InitialAngularVelocity = InAngularVelocity; }
    void SetMass(float InMass) { Mass = InMass; }
    void SetLinearDamping(float InLinearDamping) { LinearDamping = InLinearDamping; }
    void SetAngularDamping(float InAngularDamping) { AngularDamping = InAngularDamping; }
};
