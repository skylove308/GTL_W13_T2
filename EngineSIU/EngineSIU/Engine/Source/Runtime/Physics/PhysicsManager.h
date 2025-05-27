#pragma once

#include "Core/HAL/PlatformType.h" // TCHAR 재정의 문제때문에 다른 헤더들보다 앞에 있어야 함

#include <PxPhysicsAPI.h>
#include <DirectXMath.h>
#include <pvd/PxPvd.h>

#include "Container/Array.h"
#include "Container/Map.h"
#include "PhysicsEngine/PhysicsAsset.h"


enum class ERigidBodyType;
struct FBodyInstance;
class UBodySetup;
class UWorld;

using namespace physx;
using namespace DirectX;

// #define SCOPED_READ_LOCK(scene) PxSceneReadLock scopedReadLock(*scene);

class UPrimitiveComponent;

// 게임 오브젝트
struct GameObject {
    PxRigidDynamic* DynamicRigidBody = nullptr;
    PxRigidStatic* StaticRigidBody = nullptr;
    XMMATRIX WorldMatrix = XMMatrixIdentity();

    void UpdateFromPhysics(PxScene* Scene) {
        // SCOPED_READ_LOCK(gScene);
        PxTransform t = DynamicRigidBody->getGlobalPose();
        PxMat44 mat(t);
        WorldMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&mat));
    }

    void SetRigidBodyType(ERigidBodyType RigidBody) const;
};

class FPhysicsManager
{
public:
    FPhysicsManager();
    ~FPhysicsManager() = default;

    void InitPhysX();
    PxScene* CreateScene(UWorld* World);
    PxScene* GetScene(UWorld* World) { return SceneMap[World]; }
    bool ConnectPVD();
    void RemoveScene(UWorld* World) { SceneMap.Remove(World); }
    void SetCurrentScene(UWorld* World) { CurrentScene = SceneMap[World]; }
    void SetCurrentScene(PxScene* Scene) { CurrentScene = Scene; }
    
    GameObject CreateBox(const PxVec3& Pos, const PxVec3& HalfExtents) const;
    GameObject* CreateGameObject(const PxVec3& Pos, FBodyInstance* BodyInstance, UBodySetup* BodySetup, ERigidBodyType RigidBodyType =
                                     ERigidBodyType::DYNAMIC) const;

    PxShape* CreateBoxShape(const PxVec3& Pos, const PxVec3& Rotation, const PxVec3& HalfExtents) const;
    PxShape* CreateSphereShape(const PxVec3& Pos, const PxVec3& Rotation, const PxVec3& HalfExtents) const;
    PxShape* CreateCapsuleShape(const PxVec3& Pos, const PxVec3& Rotation, const PxVec3& HalfExtents) const;
    
    void Simulate(float DeltaTime);
    void ShutdownPhysX();
    void CleanupPVD();

private:
    PxDefaultAllocator      Allocator;
    PxDefaultErrorCallback  ErrorCallback;
    PxFoundation* Foundation = nullptr;
    PxPhysics* Physics = nullptr;
    TMap<UWorld*, PxScene*> SceneMap;
    PxScene* CurrentScene = nullptr;
    PxMaterial* Material = nullptr;
    PxDefaultCpuDispatcher* Dispatcher = nullptr;
    // 디버깅용
    PxPvd* Pvd;
    PxPvdTransport* Transport;

    PxRigidDynamic* CreateDynamicRigidBody(const PxVec3& Pos, FBodyInstance* BodyInstance, UBodySetup* BodySetups) const;
    PxRigidStatic* CreateStaticRigidBody(const PxVec3& Pos, FBodyInstance* BodyInstance, UBodySetup* BodySetups) const;
};

