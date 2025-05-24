#pragma once
#include <PxPhysicsAPI.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

// #include "Container/Array.h"
// #include "Math/Vector.h"

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
    GameObject CreateBox(const PxVec3& pos, const PxVec3& halfExtents) const;
    GameObject* CreateGameObject(const PxVec3& pos, const PxVec3& halfExtents) const;
    
    void Simulate(float dt);
    void ShutdownPhysX();

private:
    PxDefaultAllocator      gAllocator;
    PxDefaultErrorCallback  gErrorCallback;
    PxFoundation* gFoundation = nullptr;
    PxPhysics* gPhysics = nullptr;
    PxScene* gScene = nullptr;
    PxMaterial* gMaterial = nullptr;
    PxDefaultCpuDispatcher* gDispatcher = nullptr;
    std::vector<GameObject> gObjects;
};

