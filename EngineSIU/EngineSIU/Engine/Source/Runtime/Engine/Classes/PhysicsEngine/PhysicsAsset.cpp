
#include "PhysicsAsset.h"

#include "Engine/AssetManager.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/Casts.h"
#include "Engine/Engine.h"
#include "PhysicsManager.h"
#include "ConstraintInstance.h"

void FKAggregateGeom::SerializeShapeArray(EGeomType Type, FArchive& Ar, TArray<physx::PxShape*>& ShapeArray)
{
    int32 Num = ShapeArray.Num();
    Ar << Num;

    if (Ar.IsLoading())
    {
        /**
         * PxShape의 기본 생성자가 protected이므로, TArray의 SetNum 대신 하나씩 직접 넣음.
         */
        for (int32 i = 0; i < Num; ++i)
        {
            physx::PxVec3 Pos(0.0f, 0.0f, 0.0f);
            physx::PxVec3 Rot(0.0f, 0.0f, 0.0f);
            physx::PxVec3 Extent(0.0f, 0.0f, 0.0f);

            if (Type == EGeomType::ESphere)
            {
                ShapeArray.Add(GEngine->PhysicsManager->CreateSphereShape(Pos, Rot, Extent));
            }
            else if (Type == EGeomType::EBox)
            {
                ShapeArray.Add(GEngine->PhysicsManager->CreateBoxShape(Pos, Rot, Extent));
            }
            else if (Type == EGeomType::ECapsule)
            {
                ShapeArray.Add(GEngine->PhysicsManager->CreateCapsuleShape(Pos, Rot, Extent));
            }
        }
    }
        
    for (int32 i = 0; i < Num; ++i)
    {
        SerializeShape(Type, Ar, ShapeArray[i]);
    }
}

void FKAggregateGeom::SerializeShape(EGeomType Type, FArchive& Ar, physx::PxShape* Shape)
{
    if (Type == EGeomType::ESphere)
    {
        SerializeSphere(Ar, Shape);
    }
    else if (Type == EGeomType::EBox)
    {
        SerializeBox(Ar, Shape);
    }
    else if (Type == EGeomType::ECapsule)
    {
        SerializeCapsule(Ar, Shape);
    }
}

void FKAggregateGeom::SerializeSphere(FArchive& Ar, physx::PxShape* Shape)
{
    physx::PxTransform LocalPose = Shape->getLocalPose();
    physx::PxSphereGeometry Geometry;
    Shape->getSphereGeometry(Geometry);
        
    FVector Location = FVector(LocalPose.p.x, LocalPose.p.y, LocalPose.p.z);
    FQuat Rotation = FQuat(LocalPose.q.x, LocalPose.q.y, LocalPose.q.z, LocalPose.q.w);
    float Radius = Geometry.radius;

    Ar << Location << Rotation << Radius;

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

        Geometry.radius = Radius;
        Shape->setGeometry(Geometry);
    }
}

void FKAggregateGeom::SerializeBox(FArchive& Ar, physx::PxShape* Shape)
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

void FKAggregateGeom::SerializeCapsule(FArchive& Ar, physx::PxShape* Shape)
{
    physx::PxTransform LocalPose = Shape->getLocalPose();
    physx::PxCapsuleGeometry Geometry;
    Shape->getCapsuleGeometry(Geometry);
        
    FVector Location = FVector(LocalPose.p.x, LocalPose.p.y, LocalPose.p.z);
    FQuat Rotation = FQuat(LocalPose.q.x, LocalPose.q.y, LocalPose.q.z, LocalPose.q.w);
    float Radius = Geometry.radius;
    float HalfHeight = Geometry.halfHeight;

    Ar << Location << Rotation << Radius << HalfHeight;

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

        Geometry.radius = Radius;
        Geometry.halfHeight = HalfHeight;
        Shape->setGeometry(Geometry);
    }
}

void UBodySetupCore::SerializeAsset(FArchive& Ar)
{
    Ar << BoneName;
}

void UBodySetup::SerializeAsset(FArchive& Ar)
{
    UBodySetupCore::SerializeAsset(Ar);

    Ar << AggGeom;
}

bool UPhysicsAsset::SetPreviewMesh(USkeletalMesh* PreviewMesh)
{
    if (PreviewMesh)
    {
        for (int32 i = 0; i < BodySetups.Num(); ++i)
        {
            FName BodyName = BodySetups[i]->BoneName;
            int32 BoneIndex = PreviewMesh->GetRefSkeleton().FindBoneIndex(BodyName);
            if (BoneIndex == INDEX_NONE)
            {
                return false;
            }
        }

        PreviewSkeletalMesh = PreviewMesh;
        return true;
    }
    return false;
}

USkeletalMesh* UPhysicsAsset::GetPreviewMesh() const
{
    return PreviewSkeletalMesh;
}

void UPhysicsAsset::SerializeAsset(FArchive& Ar)
{
    FName PreviewMeshName = NAME_None;
    if (Ar.IsSaving())
    {
        PreviewMeshName = UAssetManager::Get().GetAssetKeyByObject(EAssetType::SkeletalMesh, PreviewSkeletalMesh);
    }

    Ar << PreviewMeshName;

    if (Ar.IsLoading())
    {
        if (UObject* Asset = UAssetManager::Get().GetAsset(EAssetType::SkeletalMesh, PreviewMeshName))
        {
            PreviewSkeletalMesh = Cast<USkeletalMesh>(Asset);
        }
    }

    BodySetups.SerializePtrAsset(Ar);
    ConstraintInstances.SerializePtrAsset(Ar);
}

FArchive& operator<<(FArchive& Ar, FKAggregateGeom& AggGeom)
{
    AggGeom.SerializeShapeArray(EGeomType::ESphere, Ar, AggGeom.SphereElems);
    AggGeom.SerializeShapeArray(EGeomType::EBox, Ar, AggGeom.BoxElems);
    AggGeom.SerializeShapeArray(EGeomType::ECapsule, Ar, AggGeom.CapsuleElems);
    return Ar;
}
