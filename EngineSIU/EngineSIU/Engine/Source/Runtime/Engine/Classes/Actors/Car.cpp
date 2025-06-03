#include "Car.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"
#include "Engine/FbxLoader.h"
#include "Physics/PhysicsManager.h"
#include "Lua/LuaUtils/LuaTypeMacros.h"
#include "Lua/LuaScriptComponent.h"
#include "Engine/AssetManager.h"


ACar::ACar()
{
    UStaticMeshComponent* StaticMeshComp = AddComponent<UStaticMeshComponent>("CarMesh");
    StaticMeshComp->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Cars/Train/Train.obj"));
    StaticMeshComp->bSimulate = true;
    RootComponent = StaticMeshComp;
    RootComponent->SetWorldRotation(FRotator(0.0f, 0.0f, 0.0f));
    RootComponent->SetWorldScale3D(FVector(100.0f));
}

void ACar::BeginPlay()
{
    Super::BeginPlay();
}

void ACar::PostSpawnInitialize() 
{
    Super::PostSpawnInitialize();
    LuaScriptComponent->SetScriptName("Car");
    UStaticMeshComponent* CarMeshComp = Cast<UStaticMeshComponent>(GetRootComponent());
}

void ACar::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if(GetActorLocation().Y > 9000.0f || GetActorLocation().Y < -9000.0f)
    {
        Destroy();

        return;
    }
}

UObject* ACar::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewActor->InitialVelocity = InitialVelocity; // 초기 속도 복사
    NewActor->InitialAngularVelocity = InitialAngularVelocity; // 초기 각속도 복사
    NewActor->Mass = Mass; // 질량 복사
    NewActor->LinearDamping = LinearDamping; // 선형 댐핑 복사
    NewActor->AngularDamping = AngularDamping; // 각속도 댐핑 복사

    return NewActor;
}

void ACar::RegisterLuaType(sol::state& Lua)
{
    DEFINE_LUA_TYPE_WITH_PARENT(ACar, sol::bases<AActor>(),
        "InitialVelocity", sol::property(&ThisClass::GetInitialVelocity, &ThisClass::SetInitialVelocity),
        "InitialAngularVelocity", sol::property(&ThisClass::GetInitialAngularVelocity, &ThisClass::SetInitialAngularVelocity),
        "Mass", sol::property(&ThisClass::GetMass, &ThisClass::SetMass),
        "LinearDamping", sol::property(&ThisClass::GetLinearDamping, &ThisClass::SetLinearDamping),
        "AngularDamping", sol::property(&ThisClass::GetAngularDamping, &ThisClass::SetAngularDamping),
        "SpawnDirectionRight", sol::property(&ThisClass::GetSpawnDirectionRight, &ThisClass::SetSpawnDirectionRight),
        "Drive", &ThisClass::Drive
    )
}


bool ACar::BindSelfLuaProperties()
{
    if (!Super::BindSelfLuaProperties())
        return false;

    sol::table& LuaTable = LuaScriptComponent->GetLuaSelfTable();
    if (!LuaTable.valid())
    {
        return false;
    }

    // 자기 자신 등록.
    // self에 this를 하게 되면 내부에서 임의로 Table로 바꿔버리기 때문에 self:함수() 형태의 호출이 불가능.
    // 자기 자신 객체를 따로 넘겨주어야만 AActor:GetName() 같은 함수를 실행시켜줄 수 있다.
    LuaTable["this"] = this;
    LuaTable["Name"] = *GetName(); // FString 해결되기 전까지 임시로 Table로 전달.
    // 이 아래에서 또는 하위 클래스 함수에서 멤버 변수 등록.

    return true;
}

void ACar::Drive()
{
    UStaticMeshComponent* CarMeshComp = Cast<UStaticMeshComponent>(GetRootComponent());
    if (!CarMeshComp ||
        !CarMeshComp->BodyInstance ||
        !CarMeshComp->BodyInstance->BIGameObject ||
        !CarMeshComp->BodyInstance->BIGameObject->DynamicRigidBody)
    {
        return;
    }

    auto* RigidDynamic = CarMeshComp->BodyInstance->BIGameObject->DynamicRigidBody;
    RigidDynamic->setMass(Mass); // 질량 설정
    RigidDynamic->setLinearDamping(LinearDamping); // 선형 댐핑 설정
    RigidDynamic->setAngularDamping(AngularDamping); // 각속도 댐핑 설정
    RigidDynamic->setLinearVelocity(PxVec3(InitialVelocity.X, InitialVelocity.Y, InitialVelocity.Z));
    RigidDynamic->setAngularVelocity(PxVec3(InitialAngularVelocity.X, InitialAngularVelocity.Y, InitialAngularVelocity.Z)); // 초기 각속도 설정
}
