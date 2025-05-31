#pragma once

#include "Core/HAL/PlatformType.h" // TCHAR 재정의 문제때문에 다른 헤더들보다 앞에 있어야 함

#include <PxPhysicsAPI.h>
#include <DirectXMath.h>
#include <pvd/PxPvd.h>

#include "RendererHelpers.h"
#include "Container/Array.h"
#include "Container/Map.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"


enum class ERigidBodyType : uint8;
struct FBodyInstance;
class UBodySetup;
class UWorld;

using namespace physx;
using namespace DirectX;

class UPrimitiveComponent;

struct GameObject {
    PxRigidDynamic* DynamicRigidBody = nullptr;
    PxRigidStatic* StaticRigidBody = nullptr;
    XMMATRIX WorldMatrix = XMMatrixIdentity();

    void UpdateFromPhysics(PxScene* Scene) {
        PxSceneReadLock scopedReadLock(*Scene);
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
    
    void DestroyGameObject(GameObject* GameObject) const;
    
    GameObject CreateBox(const PxVec3& Pos, const PxVec3& HalfExtents) const;
    GameObject* CreateGameObject(const PxVec3& Pos, const PxQuat& Rot, FBodyInstance* BodyInstance, UBodySetup* BodySetup, ERigidBodyType RigidBodyType =
                                     ERigidBodyType::DYNAMIC) const;
    void CreateJoint(const GameObject* Obj1, const GameObject* Obj2, FConstraintInstance* ConstraintInstance, const FConstraintSetup* ConstraintSetup) const;

    PxShape* CreateBoxShape(const PxVec3& Pos, const PxQuat& Quat, const PxVec3& HalfExtents) const;
    PxShape* CreateSphereShape(const PxVec3& Pos, const PxQuat& Quat, float Radius) const;
    PxShape* CreateCapsuleShape(const FVector& Pos, const FQuat& Quat, float Radius, float HalfHeight) const;
    PxShape* CreateCapsuleShape(const PxVec3& Pos, const PxQuat& Quat, float Radius, float HalfHeight, PxMaterial* Material = nullptr) const;
    PxQuat EulerToQuat(const PxVec3& EulerAngles) const;

    PxPhysics* GetPhysics() { return Physics; }
    PxMaterial* GetMaterial() const { return Material; }
    
    void Simulate(float DeltaTime);
    void ShutdownPhysX();
    void CleanupPVD();
    void CleanupScene();

private:
    PxDefaultAllocator Allocator;
    PxDefaultErrorCallback ErrorCallback;
    PxFoundation* Foundation = nullptr;
    PxPhysics* Physics = nullptr;
    TMap<UWorld*, PxScene*> SceneMap;
    PxScene* CurrentScene = nullptr;
    PxMaterial* Material = nullptr;
    PxDefaultCpuDispatcher* Dispatcher = nullptr;
    // 디버깅용
    PxPvd* Pvd;
    PxPvdTransport* Transport;

    PxRigidDynamic* CreateDynamicRigidBody(const PxVec3& Pos, const PxQuat& Rot, FBodyInstance* BodyInstance, UBodySetup* BodySetups) const;
    PxRigidStatic* CreateStaticRigidBody(const PxVec3& Pos, const PxQuat& Rot, FBodyInstance* BodyInstance, UBodySetup* BodySetups) const;
    void AttachShapesToActor(PxRigidActor* Actor, UBodySetup* BodySetup) const;
    void ApplyMassAndInertiaSettings(PxRigidDynamic* DynamicBody, const FBodyInstance* BodyInstance) const;
    void ApplyBodyInstanceSettings(PxRigidActor* Actor, const FBodyInstance* BodyInstance) const;
    void ApplyLockConstraints(PxRigidDynamic* DynamicBody, const FBodyInstance* BodyInstance) const;
    void ApplyCollisionSettings(const PxRigidActor* Actor, const FBodyInstance* BodyInstance) const;
    void ApplyShapeCollisionSettings(PxShape* Shape, const FBodyInstance* BodyInstance) const;
};

