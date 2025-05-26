#pragma once
#include "Container/Array.h"

#include <PxPhysicsAPI.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class USkeletalMesh;
struct FConstraintInstance;

struct FKAggregateGeom
{
    TArray<physx::PxShape*> SphereElems;
    TArray<physx::PxShape*> BoxElems;
    TArray<physx::PxShape*> CapsuleElems;
};

class UBodySetupCore : public UObject
{
    DECLARE_CLASS(UBodySetupCore, UObject)
    
public:
    UBodySetupCore() = default;

    FName BoneName;
};

class UBodySetup : public UBodySetupCore
{
    DECLARE_CLASS(UBodySetup, UBodySetupCore)
    
public:
    UBodySetup() = default;

    // DisplayName = Primitives
    FKAggregateGeom AggGeom;
};

class UPhysicsAsset : public UObject
{
    DECLARE_CLASS(UPhysicsAsset, UObject)
    
public:
    UPhysicsAsset() = default;

    USkeletalMesh* PreviewSkeletalMesh;
    
    TArray<UBodySetup*> BodySetups;

    /**
     * 언리얼 엔진에서는 UPhysicsConstraintTemplate의 배열을 가지고있고,
     * UPhysicsConstraintTemplate에는 FConstraintInstance타입인
     * DefaultInstance를 가지고있음.
     * UPhysicsConstraintTemplate의 DEPRECATED된 멤버 변수들을 보면
     * FConstraintInstance의 멤버 변수와 겹치기 때문에 사실상 같은 정보로 판단하여
     * FConstraintInstance를 사용.
     */
    TArray<FConstraintInstance*> ConstraintInstances;

    virtual bool SetPreviewMesh(USkeletalMesh* PreviewMesh);
    virtual USkeletalMesh* GetPreviewMesh() const;
};
