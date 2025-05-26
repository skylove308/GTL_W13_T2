#include "PhysicsManager.h"

#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"


FPhysicsManager::FPhysicsManager()
{
}

void FPhysicsManager::InitPhysX()
{
    Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, Allocator, ErrorCallback);
    Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, PxTolerancesScale());
    Material = Physics->createMaterial(0.5f, 0.5f, 0.6f);
}

PxScene* FPhysicsManager::CreateScene(UWorld* World)
{
    if (SceneMap[World])
    {
        return SceneMap[World];
    }
    
    PxSceneDesc sceneDesc(Physics->getTolerancesScale());
    
    sceneDesc.gravity = PxVec3(0, 0, -9.81f);
    
    Dispatcher = PxDefaultCpuDispatcherCreate(4);
    sceneDesc.cpuDispatcher = Dispatcher;
    
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    
    // sceneDesc.simulationEventCallback = gMyCallback; // TODO: 이벤트 핸들러 등록(옵저버 or component 별 override)
    
    sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
    sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
    
    PxScene* NewScene = Physics->createScene(sceneDesc);
    SceneMap.Add(World, NewScene);

    return NewScene;
}

GameObject FPhysicsManager::CreateBox(const PxVec3& Pos, const PxVec3& HalfExtents) const
{
    GameObject obj;
    PxTransform pose(Pos);
    obj.rigidBody = Physics->createRigidDynamic(pose);
    PxShape* shape = Physics->createShape(PxBoxGeometry(HalfExtents), *Material);
    obj.rigidBody->attachShape(*shape);
    PxRigidBodyExt::updateMassAndInertia(*obj.rigidBody, 10.0f);
    CurrentScene->addActor(*obj.rigidBody);
    obj.UpdateFromPhysics(CurrentScene);
    return obj;
}

GameObject* FPhysicsManager::CreateGameObject(const PxVec3& Pos, FBodyInstance* BodyInstance, TArray<UBodySetup*> BodySetups) const
{
    GameObject* Obj = new GameObject();
    const PxTransform Pose(Pos);
    Obj->rigidBody = Physics->createRigidDynamic(Pose);
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
    CurrentScene->addActor(*Obj->rigidBody);
    Obj->UpdateFromPhysics(CurrentScene);
    Obj->rigidBody->userData = (void*)BodyInstance;
    
    return Obj;
}

PxShape* FPhysicsManager::CreateBoxShape(const PxVec3& Pos, const PxVec3& Rotation, const PxVec3& HalfExtents) const
{
    PxShape* Result = Physics->createShape(PxBoxGeometry(HalfExtents), *Material);
    PxTransform localPose(Pos);
    Result->setLocalPose(localPose);
    return Result;
}

PxShape* FPhysicsManager::CreateSphereShape(const PxVec3& Pos, const PxVec3& Rotation, const PxVec3& HalfExtents) const
{
    PxShape* Result = Physics->createShape(PxSphereGeometry(HalfExtents.x), *Material);
    PxTransform localPose(Pos);
    Result->setLocalPose(localPose);
    return Result;
}

PxShape* FPhysicsManager::CreateCapsuleShape(const PxVec3& Pos, const PxVec3& Rotation, const PxVec3& HalfExtents) const
{
    PxShape* Result = Physics->createShape(PxCapsuleGeometry(HalfExtents.x, HalfExtents.z), *Material);
    PxTransform localPose(Pos);
    Result->setLocalPose(localPose);
    return Result;
}

void FPhysicsManager::Simulate(float DeltaTime)
{
    if (CurrentScene)
    {
        CurrentScene->simulate(DeltaTime);
        CurrentScene->fetchResults(true);
    }
}

void FPhysicsManager::ShutdownPhysX()
{
    if(CurrentScene)
    {
        CurrentScene->release();
        CurrentScene = nullptr;
    }
    if(Dispatcher)
    {
        Dispatcher->release();
        Dispatcher = nullptr;
    }
    if(Material)
    {
        Material->release();
        Material = nullptr;
    }
    if(Physics)
    {
        Physics->release();
        Physics = nullptr;
    }
    if(Foundation)
    {
        Foundation->release();
        Foundation = nullptr;
    }
}
