#pragma once

#include "Core/HAL/PlatformType.h" // TCHAR 재정의 문제때문에 다른 헤더들보다 앞에 있어야 함
#include "World/World.h"

#include <PxPhysicsAPI.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>


class UBodySetup;

using namespace physx;
using namespace DirectX;

// #define SCOPED_READ_LOCK(scene) PxSceneReadLock scopedReadLock(*scene);

class UPrimitiveComponent;

// 게임 오브젝트
struct GameObject {
    PxRigidDynamic* rigidBody = nullptr;
    XMMATRIX worldMatrix = XMMatrixIdentity();

    void UpdateFromPhysics(PxScene* gScene) {
        // SCOPED_READ_LOCK(gScene);
        PxTransform t = rigidBody->getGlobalPose();
        PxMat44 mat(t);
        worldMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&mat));
    }
};

class FPhysicsManager
{
public:
    FPhysicsManager();
    ~FPhysicsManager() = default;

    void InitPhysX();
    PxScene* CreateScene(UWorld* World);
    PxScene* GetScene(UWorld* World) { return SceneMap[World]; }
    void RemoveScene(UWorld* World) { SceneMap.Remove(World); }
    void SetCurrentScene(UWorld* World) { gScene = SceneMap[World]; }
    void SetCurrentScene(PxScene* Scene) { gScene = Scene; }
    
    GameObject CreateBox(const PxVec3& pos, const PxVec3& halfExtents) const;
    GameObject* CreateGameObject(const PxVec3& pos, const PxVec3& halfExtents, FBodyInstance* BodyInstance) const;
    
    void Simulate(float dt);
    void ShutdownPhysX();

private:
    PxDefaultAllocator      gAllocator;
    PxDefaultErrorCallback  gErrorCallback;
    PxFoundation* gFoundation = nullptr;
    PxPhysics* gPhysics = nullptr;
    TMap<UWorld*, PxScene*> SceneMap;
    PxScene* gScene = nullptr;
    PxMaterial* gMaterial = nullptr;
    PxDefaultCpuDispatcher* gDispatcher = nullptr;
    std::vector<GameObject> gObjects;
};

