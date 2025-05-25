#include "BodyInstance.h"
#include "Components/PrimitiveComponent.h"
#include "Physics/PhysicsManager.h"


FBodyInstance::FBodyInstance(UPrimitiveComponent* InOwner) : OwnerComponent(InOwner)
{
    
}

void FBodyInstance::SetGameObject(GameObject* InGameObject)
{
    BIGameObject = InGameObject;
}

