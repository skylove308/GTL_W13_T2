#include "BodyInstance.h"
#include "Components/PrimitiveComponent.h"
#include "Physics/PhysicsManager.h"


FBodyInstance::FBodyInstance(UPrimitiveComponent* InOwner) : OwnerComponent(InOwner)
{
    CollisionEnabled = ECollisionEnabled::QueryAndPhysics;  // 물리와 쿼리 모두 활성화
    bUseCCD = true;                                        // CCD 활성화
    bStartAwake = true;
}

void FBodyInstance::SetGameObject(GameObject* InGameObject)
{
    BIGameObject = InGameObject;
}

