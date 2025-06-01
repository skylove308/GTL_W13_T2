#include "SimulationEventCallback.h"
#include <PhysicsEngine/BodyInstance.h>
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"

void FSimulationEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
    physx::PxRigidActor* pxActor0 = pairHeader.actors[0];
    physx::PxRigidActor* pxActor1 = pairHeader.actors[1];

    if (!pxActor0 || !pxActor1)
    {
        return;
    }

    FBodyInstance* bodyInstance0 = static_cast<FBodyInstance*>(pxActor0->userData);
    FBodyInstance* bodyInstance1 = static_cast<FBodyInstance*>(pxActor1->userData);

    UPrimitiveComponent* primComp0 = bodyInstance0 ? bodyInstance0->OwnerComponent : nullptr;
    UPrimitiveComponent* primComp1 = bodyInstance1 ? bodyInstance1->OwnerComponent : nullptr;

    if (!primComp0 || !primComp1)
    {
        return;
    }

    AActor* ownerActor0 = primComp0->GetOwner();
    AActor* ownerActor1 = primComp1->GetOwner();

    for (physx::PxU32 i = 0; i < nbPairs; ++i)
    {
        const physx::PxContactPair& pair = pairs[i]; // pair.flags는 PxContactPairFlags (PxContactPairFlag::Enum 기반)

        // --- primComp0 관점에서의 FHitResult 준비 ---
        FHitResult hitResultForPrimComp0;
        hitResultForPrimComp0.HitActor = ownerActor1;
        hitResultForPrimComp0.Component = primComp1;

        const char* shape0Name = pair.shapes[0] ? pair.shapes[0]->getName() : nullptr;
        const char* shape1Name = pair.shapes[1] ? pair.shapes[1]->getName() : nullptr;
        if (shape0Name) hitResultForPrimComp0.MyBoneName = FName(shape0Name);
        if (shape1Name) hitResultForPrimComp0.BoneName = FName(shape1Name);

        // pairHeader.flags (PxContactPairHeaderFlag::Enum 기반)를 사용하여 bBlockingHit 설정
        // 관련된 액터 중 하나라도 제거되었다면, 유효한 '블로킹' 충돌로 간주하지 않을 수 있습니다.
        hitResultForPrimComp0.bBlockingHit = !(pairHeader.flags & (physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_0 | physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_1));

        float maxPenetrationThisPair = 0.f;
        bool bFoundContactPointDetail = false; // PxContactStreamIterator로부터 상세 정보를 얻었는지 여부

        // PxContactStreamIterator를 사용하여 상세 접촉점 정보에 접근합니다.
        // 이 코드가 올바르게 작동하려면, 필터 셰이더에서 해당 충돌 쌍에 대해
        // PxPairFlag::eNOTIFY_CONTACT_POINTS 플래그가 설정되어 있어야 합니다.
        // 또한, 실제 접촉점(pair.contactCount > 0)이 있어야 반복자가 의미 있는 데이터를 제공합니다.
        if (pair.contactCount > 0)
        {
            physx::PxContactStreamIterator iter(pair.contactPatches, pair.contactPoints, pair.getInternalFaceIndices(), pair.patchCount, pair.contactCount);
            while (iter.hasNextPatch())
            {
                iter.nextPatch();
                // const physx::PxU32 faceIndexOnShape0 = iter.getFaceIndex0(); // primComp0의 shape에 대한 면 인덱스
                const physx::PxU32 faceIndexOnShape1 = iter.getFaceIndex1(); // primComp1의 shape에 대한 면 인덱스

                while (iter.hasNextContact())
                {
                    iter.nextContact();
                    if (!bFoundContactPointDetail) // 첫 번째 유효한 접촉점 정보 사용
                    {
                        hitResultForPrimComp0.ImpactPoint = PxVec3ToFVector(iter.getContactPoint());
                        hitResultForPrimComp0.ImpactNormal = PxVec3ToFVector(iter.getContactNormal());
                        hitResultForPrimComp0.Normal = -hitResultForPrimComp0.ImpactNormal; // 충돌을 당한 입장의 Normal
                        hitResultForPrimComp0.Location = hitResultForPrimComp0.ImpactPoint; // 단순 접촉 시 동일
                        hitResultForPrimComp0.FaceIndex = (faceIndexOnShape1 != 0xFFFFFFFF) ? faceIndexOnShape1 : -1;
                        bFoundContactPointDetail = true;
                    }
                    if (iter.getSeparation() < 0.f) // 음수이면 관통
                    {
                        hitResultForPrimComp0.bStartPenetrating = true;
                        float currentPen = -iter.getSeparation();
                        if (currentPen > maxPenetrationThisPair)
                        {
                            maxPenetrationThisPair = currentPen;
                        }
                    }
                }
            }
            if (bFoundContactPointDetail) { // 반복자로부터 정보를 얻었다면 PenetrationDepth 설정
                hitResultForPrimComp0.PenetrationDepth = maxPenetrationThisPair;
            }
        }
        // 만약 필터 셰이더에서 eNOTIFY_CONTACT_POINTS가 설정되지 않았거나 contactCount가 0이면,
        // bFoundContactPointDetail은 false로 유지되며, FHitResult의 관련 필드들은 기본값으로 남습니다.

        // --- primComp1 관점에서의 FHitResult 준비 ---
        FHitResult hitResultForPrimComp1 = FHitResult::GetReversedHit(hitResultForPrimComp0);
        hitResultForPrimComp1.HitActor = ownerActor0;
        hitResultForPrimComp1.Component = primComp0;

        if (bFoundContactPointDetail) { // 상세 정보를 얻었다면, primComp1 관점의 FaceIndex 설정
            // 첫 번째 반복에서 faceIndexOnShape0 값을 저장해두거나, PxContactStreamIterator를 다시 사용해야 합니다.
            // 여기서는 간결함을 위해 다시 반복하는 형태로 두었지만, 실제로는 저장/재사용 방식을 선택해야 합니다.
            physx::PxContactStreamIterator iterForFaceIndex(pair.contactPatches, pair.contactPoints, pair.getInternalFaceIndices(), pair.patchCount, pair.contactCount);
            if (iterForFaceIndex.hasNextPatch()) {
                iterForFaceIndex.nextPatch();
                hitResultForPrimComp1.FaceIndex = (iterForFaceIndex.getFaceIndex0() != 0xFFFFFFFF) ? iterForFaceIndex.getFaceIndex0() : -1;
            }
            else {
                hitResultForPrimComp1.FaceIndex = -1;
            }
        }
        else {
            hitResultForPrimComp1.FaceIndex = -1; // 상세 정보가 없다면 기본값
        }

        // --- 이벤트 발송 (pair.events는 PxPairFlag::Enum 기반) ---
        if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
        {
            if (primComp0) primComp0->OnCollisionEnter(primComp0, primComp1, hitResultForPrimComp0);
            if (primComp1) primComp1->OnCollisionEnter(primComp1, primComp0, hitResultForPrimComp1);
        }

        if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
        {
            if (primComp0) primComp0->OnCollisionStay(primComp0, ownerActor1, primComp1, hitResultForPrimComp0);
            if (primComp1) primComp1->OnCollisionStay(primComp1, ownerActor0, primComp0, hitResultForPrimComp1);
        }

        if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
        {
            if (primComp0) primComp0->OnCollisionExit(primComp0, primComp1);
            if (primComp1) primComp1->OnCollisionExit(primComp1, primComp0);
        }
    }
}
void FSimulationEventCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
}

void FSimulationEventCallback::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
{
}

void FSimulationEventCallback::onWake(physx::PxActor** actors, physx::PxU32 count)
{
}

void FSimulationEventCallback::onSleep(physx::PxActor** actors, physx::PxU32 count)
{
}

FVector FSimulationEventCallback::PxVec3ToFVector(const physx::PxVec3& vec)
{
    return FVector(vec.x, vec.y, vec.z);
}

void FSimulationEventCallback::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
{
}
