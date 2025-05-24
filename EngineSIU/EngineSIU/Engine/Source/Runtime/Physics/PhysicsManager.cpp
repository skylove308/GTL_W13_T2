#include "PhysicsManager.h"

FPhysicsManager::FPhysicsManager()
{
}

void FPhysicsManager::InitPhysX()
{
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale());
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0, -9.81f, 0);
    gDispatcher = PxDefaultCpuDispatcherCreate(4);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    // sceneDesc.simulationEventCallback = gMyCallback; // TODO: 이벤트 핸들러 등록(옵저버 or component 별 override)
    sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
    sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
    gScene = gPhysics->createScene(sceneDesc);
}

GameObject FPhysicsManager::CreateBox(const PxVec3& pos, const PxVec3& halfExtents) const
{
    GameObject obj;
    PxTransform pose(pos);
    obj.rigidBody = gPhysics->createRigidDynamic(pose);
    PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtents), *gMaterial);
    obj.rigidBody->attachShape(*shape);
    PxRigidBodyExt::updateMassAndInertia(*obj.rigidBody, 10.0f);
    gScene->addActor(*obj.rigidBody);
    obj.UpdateFromPhysics(gScene);
    return obj;
}

GameObject* FPhysicsManager::CreateGameObject(const PxVec3& pos, const PxVec3& halfExtents) const
{
    GameObject* obj = new GameObject();
    const PxTransform pose(pos);
    obj->rigidBody = gPhysics->createRigidDynamic(pose);
    PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtents), *gMaterial);
    obj->rigidBody->attachShape(*shape);
    PxRigidBodyExt::updateMassAndInertia(*obj->rigidBody, 10.0f);
    gScene->addActor(*obj->rigidBody);
    obj->UpdateFromPhysics(gScene);
    return obj;
}

void FPhysicsManager::Simulate(float dt)
{
    gScene->simulate(dt);
    gScene->fetchResults(true);
    for (auto& obj : gObjects) obj.UpdateFromPhysics(gScene);
}

void FPhysicsManager::ShutdownPhysX()
{
    gScene->release();
    gDispatcher->release();
    gMaterial->release();
    gPhysics->release();
    gFoundation->release();
    gObjects.clear();
}
