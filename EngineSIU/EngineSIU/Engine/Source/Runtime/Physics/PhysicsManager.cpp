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
    
    PxSceneDesc SceneDesc(Physics->getTolerancesScale());
    
    SceneDesc.gravity = PxVec3(0, 0, -9.81f);
    
    Dispatcher = PxDefaultCpuDispatcherCreate(4);
    SceneDesc.cpuDispatcher = Dispatcher;
    
    SceneDesc.filterShader = PxDefaultSimulationFilterShader;
    
    // sceneDesc.simulationEventCallback = gMyCallback; // TODO: 이벤트 핸들러 등록(옵저버 or component 별 override)
    
    SceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    SceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
    SceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
    
    PxScene* NewScene = Physics->createScene(SceneDesc);
    SceneMap.Add(World, NewScene);

    return NewScene;
}

GameObject FPhysicsManager::CreateBox(const PxVec3& Pos, const PxVec3& HalfExtents) const
{
    GameObject Obj;
    
    PxTransform Pose(Pos);
    Obj.RigidBody = Physics->createRigidDynamic(Pose);
    
    PxShape* Shape = Physics->createShape(PxBoxGeometry(HalfExtents), *Material);
    Obj.RigidBody->attachShape(*Shape);
    
    PxRigidBodyExt::updateMassAndInertia(*Obj.RigidBody, 10.0f);
    CurrentScene->addActor(*Obj.RigidBody);
    
    Obj.UpdateFromPhysics(CurrentScene);
    
    return Obj;
}

GameObject* FPhysicsManager::CreateGameObject(const PxVec3& Pos, FBodyInstance* BodyInstance, TArray<UBodySetup*> BodySetups) const
{
    GameObject* Obj = new GameObject();
    
    const PxTransform Pose(Pos);
    Obj->RigidBody = Physics->createRigidDynamic(Pose);
    
    for (const auto& BodySetup : BodySetups)
    {
        for (const auto& Sphere : BodySetup->AggGeom.SphereElems)
        {
            Obj->RigidBody->attachShape(*Sphere);
        }

        for (const auto& Box : BodySetup->AggGeom.BoxElems)
        {
            Obj->RigidBody->attachShape(*Box);
        }

        for (const auto& Capsule : BodySetup->AggGeom.CapsuleElems)
        {
            Obj->RigidBody->attachShape(*Capsule);
        }
    }
    
    PxRigidBodyExt::updateMassAndInertia(*Obj->RigidBody, 10.0f);
    
    CurrentScene->addActor(*Obj->RigidBody);
    
    Obj->UpdateFromPhysics(CurrentScene);
    Obj->RigidBody->userData = static_cast<void*>(BodyInstance);
    
    return Obj;
}

void FPhysicsManager::DestroyGameObject(GameObject* GameObject) const
{
    if (GameObject && GameObject->RigidBody)
    {
        CurrentScene->removeActor(*GameObject->RigidBody);
        GameObject->RigidBody->release();
        GameObject->RigidBody = nullptr;
    }
    delete GameObject;
}

PxShape* FPhysicsManager::CreateBoxShape(const PxVec3& Pos, const PxVec3& Rotation, const PxVec3& HalfExtents) const
{
    PxShape* Result = Physics->createShape(PxBoxGeometry(HalfExtents), *Material);
    PxTransform LocalPos(Pos);
    Result->setLocalPose(LocalPos);
    return Result;
}

PxShape* FPhysicsManager::CreateSphereShape(const PxVec3& Pos, const PxVec3& Rotation, const PxVec3& HalfExtents) const
{
    PxShape* Result = Physics->createShape(PxSphereGeometry(HalfExtents.x), *Material);
    PxTransform LocalPos(Pos);
    Result->setLocalPose(LocalPos);
    return Result;
}

PxShape* FPhysicsManager::CreateCapsuleShape(const PxVec3& Pos, const PxVec3& Rotation, const PxVec3& HalfExtents) const
{
    PxShape* Result = Physics->createShape(PxCapsuleGeometry(HalfExtents.x, HalfExtents.z), *Material);
    PxTransform LocalPos(Pos);
    Result->setLocalPose(LocalPos);
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
