#pragma once
#include "GameFramework/Actor.h"

enum class ECarType : uint8
{
    Benz,
    RangeRover,
    Truck,
    Train
};

class ACar : public AActor
{
    DECLARE_CLASS(ACar, AActor)
public:
    ACar();
    virtual void BeginPlay() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void PostSpawnInitialize() override;
    virtual void Tick(float DeltaTime) override;

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
    
    bool SpawnDirectionRight; // 차량이 생성될 방향을 결정하는 변수 (true: 오른쪽, false: 왼쪽)
    ECarType CarType; // 차량의 종류를 나타내는 변수

public:
    //getter
    FVector GetInitialVelocity() const { return InitialVelocity; }
    FVector GetInitialAngularVelocity() const { return InitialAngularVelocity; }
    float GetMass() const { return Mass; }
    float GetLinearDamping() const { return LinearDamping; }
    float GetAngularDamping() const { return AngularDamping; }
    bool GetSpawnDirectionRight() const { return SpawnDirectionRight; }
    ECarType GetCarType() const { return CarType; } // 차량 종류를 반환하는 함수

    //setter
    void SetInitialVelocity(const FVector& InVelocity) { InitialVelocity = InVelocity; }
    void SetInitialAngularVelocity(const FVector& InAngularVelocity) { InitialAngularVelocity = InAngularVelocity; }
    void SetMass(float InMass) { Mass = InMass; }
    void SetLinearDamping(float InLinearDamping) { LinearDamping = InLinearDamping; }
    void SetAngularDamping(float InAngularDamping) { AngularDamping = InAngularDamping; }
    void SetSpawnDirectionRight(bool InSpawnDirection) { SpawnDirectionRight = InSpawnDirection; } // 차량 생성 방향 설정 함수
    void SetCarType(ECarType InCarType) { CarType = InCarType; } // 차량 종류 설정 함수
};
