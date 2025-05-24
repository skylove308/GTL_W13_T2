#pragma once
#include <PxPhysicsAPI.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

using namespace physx;
using namespace DirectX;

// 게임 오브젝트
struct GameObject {
    PxRigidDynamic* rigidBody = nullptr;
    XMMATRIX worldMatrix = XMMatrixIdentity();

    void UpdateFromPhysics() {
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
    GameObject CreateBox(const PxVec3& pos, const PxVec3& halfExtents);
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

