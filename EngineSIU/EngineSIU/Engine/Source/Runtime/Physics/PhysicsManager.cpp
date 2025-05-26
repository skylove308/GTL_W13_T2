#include "PhysicsManager.h"

#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"

#include "World/World.h"

void GameObject::SetRigidBodyType(ERigidBodyType RigidBodyType) const
{
    switch (RigidBodyType)
    {
    case ERigidBodyType::STATIC:
    {
        // 재생성해야 함
        break;
    }
        case ERigidBodyType::DYNAMIC:
    {
        DynamicRigidBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
        break;
    }
        case ERigidBodyType::KINEMATIC:
    {
        DynamicRigidBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
        DynamicRigidBody->setLinearVelocity(PxVec3(0));
        DynamicRigidBody->setAngularVelocity(PxVec3(0));
        break;
    }
    }
}

FPhysicsManager::FPhysicsManager()
{
}

void FPhysicsManager::InitPhysX()
{
    Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, Allocator, ErrorCallback);

    // PVD 생성 및 연결
    Pvd = PxCreatePvd(*Foundation);
    if (Pvd) {
        // TCP 연결 (기본 포트 5425)
        Transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
        if (Transport) {
            Pvd->connect(*Transport, PxPvdInstrumentationFlag::eALL);
        }
    }
    
    Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, PxTolerancesScale(), true, Pvd);
    
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

    // PVD 클라이언트 생성 및 씬 연결
    if (Pvd && Pvd->isConnected()) {
        PxPvdSceneClient* pvdClient = NewScene->getScenePvdClient();
        if (pvdClient) {
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
        }
    }

    return NewScene;
}

bool FPhysicsManager::ConnectPVD()
{
    // PVD 생성
    Pvd = PxCreatePvd(*Foundation);
    if (!Pvd) {
        printf("PVD 생성 실패\n");
        return false;
    }

    // 네트워크 전송 생성 (TCP)
    Transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    if (!Transport) {
        printf("PVD Transport 생성 실패\n");
        return false;
    }

    // PVD 연결 및 계측 플래그 설정
    bool connected = Pvd->connect(*Transport, 
        PxPvdInstrumentationFlag::eALL);  // 모든 데이터 전송
    // 또는 특정 플래그만:
    // PxPvdInstrumentationFlag::eDEBUG | 
    // PxPvdInstrumentationFlag::ePROFILE |
    // PxPvdInstrumentationFlag::eMEMORY

    if (connected) {
        printf("PVD 연결 성공\n");
    } else {
        printf("PVD 연결 실패\n");
    }

    return connected;
}

GameObject FPhysicsManager::CreateBox(const PxVec3& Pos, const PxVec3& HalfExtents) const
{
    GameObject obj;
    PxTransform pose(Pos);
    obj.DynamicRigidBody = Physics->createRigidDynamic(pose);
    PxShape* shape = Physics->createShape(PxBoxGeometry(HalfExtents), *Material);
    obj.DynamicRigidBody->attachShape(*shape);
    PxRigidBodyExt::updateMassAndInertia(*obj.DynamicRigidBody, 10.0f);
    CurrentScene->addActor(*obj.DynamicRigidBody);
    obj.UpdateFromPhysics(CurrentScene);
    return obj;
}

GameObject* FPhysicsManager::CreateGameObject(const PxVec3& Pos, FBodyInstance* BodyInstance, const TArray<UBodySetup*>& BodySetups,ERigidBodyType RigidBodyType) const
{
    GameObject* Obj = new GameObject();
    
    // RigidBodyType에 따라 다른 타입의 Actor 생성
    switch (RigidBodyType)
    {
    case ERigidBodyType::STATIC:
    {
        Obj->StaticRigidBody = CreateStaticRigidBody(Pos, BodyInstance, BodySetups);
        break;
    }
    case ERigidBodyType::DYNAMIC:
    {
        Obj->DynamicRigidBody = CreateDynamicRigidBody(Pos, BodyInstance, BodySetups);
        break;
    }
    case ERigidBodyType::KINEMATIC:
    {
        Obj->DynamicRigidBody = CreateDynamicRigidBody(Pos, BodyInstance, BodySetups);
        Obj->DynamicRigidBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
        break;
    }
    }
    
    return Obj;
}

PxRigidDynamic* FPhysicsManager::CreateDynamicRigidBody(const PxVec3& Pos, FBodyInstance* BodyInstance, TArray<UBodySetup*> BodySetups) const
{
    const PxTransform Pose(Pos);
    PxRigidDynamic* DynamicRigidBody = Physics->createRigidDynamic(Pose);

    for (const auto& BodySetup : BodySetups)
    {
        for (const auto& Sphere : BodySetup->AggGeom.SphereElems)
        {
            DynamicRigidBody->attachShape(*Sphere);
        }

        for (const auto& Box : BodySetup->AggGeom.BoxElems)
        {
            DynamicRigidBody->attachShape(*Box);
        }

        for (const auto& Capsule : BodySetup->AggGeom.CapsuleElems)
        {
            DynamicRigidBody->attachShape(*Capsule);
        }
    }

    PxRigidBodyExt::updateMassAndInertia(*DynamicRigidBody, 10.0f);
    CurrentScene->addActor(*DynamicRigidBody); // 여기에 넣을지 CreateGameObject에 넣을지 고민해보기
    DynamicRigidBody->userData = (void*)BodyInstance;

    return DynamicRigidBody;
}

PxRigidStatic* FPhysicsManager::CreateStaticRigidBody(const PxVec3& Pos, FBodyInstance* BodyInstance, TArray<UBodySetup*> BodySetups) const
{
    const PxTransform Pose(Pos);
    PxRigidStatic* StaticRigidBody = Physics->createRigidStatic(Pose);
    
    for (const auto& BodySetup : BodySetups)
    {
        for (const auto& Sphere : BodySetup->AggGeom.SphereElems)
        {
            StaticRigidBody->attachShape(*Sphere);
        }

        for (const auto& Box : BodySetup->AggGeom.BoxElems)
        {
            StaticRigidBody->attachShape(*Box);
        }

        for (const auto& Capsule : BodySetup->AggGeom.CapsuleElems)
        {
            StaticRigidBody->attachShape(*Capsule);
        }
    }

    CurrentScene->addActor(*StaticRigidBody); // 여기에 넣을지 CreateGameObject에 넣을지 고민해보기
    StaticRigidBody->userData = (void*)BodyInstance;

    return StaticRigidBody;
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

void FPhysicsManager::CleanupPVD() {
    if (Pvd) {
        if (Pvd->isConnected()) {
            Pvd->disconnect();
        }
        if (Transport) {
            Transport->release();
            Transport = nullptr;
        }
        Pvd->release();
        Pvd = nullptr;
    }
}
