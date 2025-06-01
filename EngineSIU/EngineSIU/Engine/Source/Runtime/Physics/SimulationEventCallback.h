#pragma once
#include <PxPhysicsAPI.h>
#include "Math/Vector.h"


class FSimulationEventCallback : public physx::PxSimulationEventCallback
{
public:
    FSimulationEventCallback() {}
    virtual ~FSimulationEventCallback() {}

    // 충돌
    virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
    // 트리거
    virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
    // 제약 조건 파괴 이벤트
    virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
    // 액터 슬립/웨이크
    virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override;
    virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override;
    virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override;

private:
    FVector PxVec3ToFVector(const physx::PxVec3& vec);


    // PxSimulationEventCallback을(를) 통해 상속됨

};
