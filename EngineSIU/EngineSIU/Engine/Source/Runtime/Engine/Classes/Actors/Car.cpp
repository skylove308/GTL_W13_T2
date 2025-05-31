#include "Car.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"
#include "Physics/PhysicsManager.h"

ACar::ACar()
{
    UStaticMeshComponent* StaticMeshComp = AddComponent<UStaticMeshComponent>("CarMesh");
    StaticMeshComp->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Primitives/CubePrimitive.Obj"));
    RootComponent = StaticMeshComp;
}

void ACar::BeginPlay()
{
    Super::BeginPlay();
    UStaticMeshComponent* CarMeshComp = Cast<UStaticMeshComponent>(GetRootComponent());
    if (!CarMeshComp ||
        !CarMeshComp->BodyInstance ||
        !CarMeshComp->BodyInstance->BIGameObject ||
        !CarMeshComp->BodyInstance->BIGameObject->DynamicRigidBody)
    {
        return;
    }

    auto* RigidDynamic = CarMeshComp->BodyInstance->BIGameObject->DynamicRigidBody;
    RigidDynamic->setMass(1000.0f); // 질량 설정
    RigidDynamic->setLinearDamping(0.1f); // 선형 댐핑 설정
    RigidDynamic->setAngularDamping(0.1f); // 각속도 댐핑 설정
    RigidDynamic->setLinearVelocity(PxVec3(0, -1000.0f, 0.f));
    RigidDynamic->setAngularVelocity(PxVec3(0, 0, 0)); // 초기 각속도 설정
}
