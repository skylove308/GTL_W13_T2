#include "PhysicsManager.h"

#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"


FPhysicsManager::FPhysicsManager()
{
}

void FPhysicsManager::InitPhysX()
{
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale());
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
}

PxScene* FPhysicsManager::CreateScene(UWorld* World)
{
    if (SceneMap[World])
    {
        return SceneMap[World];
    }
    
    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    
    sceneDesc.gravity = PxVec3(0, -9.81f, 0);
    
    gDispatcher = PxDefaultCpuDispatcherCreate(4);
    sceneDesc.cpuDispatcher = gDispatcher;
    
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    
    // sceneDesc.simulationEventCallback = gMyCallback; // TODO: 이벤트 핸들러 등록(옵저버 or component 별 override)
    
    sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
    sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
    
    PxScene* NewScene = gPhysics->createScene(sceneDesc);
    SceneMap.Add(World, NewScene);

    return NewScene;
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

GameObject* FPhysicsManager::CreateGameObject(const PxVec3& Pos, FBodyInstance* BodyInstance, TArray<UBodySetup*> BodySetups) const
{
    GameObject* Obj = new GameObject();
    const PxTransform Pose(Pos);
    Obj->rigidBody = gPhysics->createRigidDynamic(Pose);
    for (const auto& BodySetup : BodySetups)
    {
        for (const auto& Sphere : BodySetup->AggGeom.SphereElems)
        {
            Obj->rigidBody->attachShape(*Sphere);
        }

        for (const auto& Box : BodySetup->AggGeom.BoxElems)
        {
            Obj->rigidBody->attachShape(*Box);
        }

        for (const auto& Capsule : BodySetup->AggGeom.CapsuleElems)
        {
            Obj->rigidBody->attachShape(*Capsule);
        }
    }
    
    PxRigidBodyExt::updateMassAndInertia(*Obj->rigidBody, 10.0f);
    gScene->addActor(*Obj->rigidBody);
    Obj->UpdateFromPhysics(gScene);
    Obj->rigidBody->userData = (void*)BodyInstance;
    
    return Obj;
}

PxShape* FPhysicsManager::CreateBoxShape(const PxVec3& pos, const PxVec3& halfExtents) const
{
    PxShape* Result = gPhysics->createShape(PxSphereGeometry(halfExtents.x), *gMaterial);
    PxTransform localPose(pos);
    Result->setLocalPose(localPose);
    return Result;
}

PxShape* FPhysicsManager::CreateSphereShape(const PxVec3& pos, const PxVec3& halfExtents) const
{
    PxShape* Result = gPhysics->createShape(PxBoxGeometry(halfExtents), *gMaterial);
    PxTransform localPose(pos);
    Result->setLocalPose(localPose);
    return Result;
}

PxShape* FPhysicsManager::CreateCapsuleShape(const PxVec3& pos, const PxVec3& halfExtents) const
{
    PxShape* Result = gPhysics->createShape(PxCapsuleGeometry(halfExtents.x, halfExtents.z), *gMaterial);
    PxTransform localPose(pos);
    Result->setLocalPose(localPose);
    return Result;
}

void FPhysicsManager::Simulate(float dt)
{
    if (gScene)
    {
        gScene->simulate(dt);
        gScene->fetchResults(true);
        for (auto& obj : gObjects)
        {
            obj.UpdateFromPhysics(gScene);
        }
    }
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
