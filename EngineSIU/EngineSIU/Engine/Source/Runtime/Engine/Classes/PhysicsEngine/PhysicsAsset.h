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

    friend FArchive& operator<<(FArchive& Ar, FKAggregateGeom& AggGeom)
    {
        AggGeom.SerializeShapeArray(Ar, AggGeom.SphereElems);
        AggGeom.SerializeShapeArray(Ar, AggGeom.BoxElems);
        AggGeom.SerializeShapeArray(Ar, AggGeom.CapsuleElems);
        return Ar;
    }

private:
    void SerializeShapeArray(FArchive& Ar, TArray<physx::PxShape*>& ShapeArray)
    {
        int32 Num = ShapeArray.Num();
        Ar << Num;

        if (Ar.IsLoading())
        {
            ShapeArray.SetNum(Num);
        }
        
        for (int32 i = 0; i < Num; ++i)
        {
            SerializeShape(Ar, ShapeArray[i]);
        }
    }

    void SerializeShape(FArchive& Ar, physx::PxShape* Shape)
    {
        physx::PxTransform LocalPose = Shape->getLocalPose();
        physx::PxBoxGeometry Geometry;
        Shape->getBoxGeometry(Geometry);
        
        FVector Location = FVector(LocalPose.p.x, LocalPose.p.y, LocalPose.p.z);
        FQuat Rotation = FQuat(LocalPose.q.x, LocalPose.q.y, LocalPose.q.z, LocalPose.q.w);
        FVector HalfExtent = FVector(Geometry.halfExtents.x, Geometry.halfExtents.y, Geometry.halfExtents.z);

        Ar << Location << Rotation << HalfExtent;

        if (Ar.IsLoading())
        {
            LocalPose.p.x = Location.X;
            LocalPose.p.y = Location.Y;
            LocalPose.p.z = Location.Z;
            LocalPose.q.x = Rotation.X;
            LocalPose.q.y = Rotation.Y;
            LocalPose.q.z = Rotation.Z;
            LocalPose.q.w = Rotation.W;
            Shape->setLocalPose(LocalPose);

            Geometry.halfExtents.x = HalfExtent.X;
            Geometry.halfExtents.y = HalfExtent.Y;
            Geometry.halfExtents.z = HalfExtent.Z;
            Shape->setGeometry(Geometry);
        }
    }
};

class UBodySetupCore : public UObject
{
    DECLARE_CLASS(UBodySetupCore, UObject)
    
public:
    UBodySetupCore() = default;

    FName BoneName;

    virtual void SerializeAsset(FArchive& Ar) override;
};

class UBodySetup : public UBodySetupCore
{
    DECLARE_CLASS(UBodySetup, UBodySetupCore)
    
public:
    UBodySetup() = default;

    // DisplayName = Primitives
    FKAggregateGeom AggGeom;

    virtual void SerializeAsset(FArchive& Ar) override;
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

    virtual void SerializeAsset(FArchive& Ar) override;
};
