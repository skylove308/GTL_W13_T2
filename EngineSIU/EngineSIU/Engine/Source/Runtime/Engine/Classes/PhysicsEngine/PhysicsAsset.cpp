
#include "PhysicsAsset.h"

#include "Engine/AssetManager.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/Casts.h"

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
    FName SkeletalMeshName = NAME_None;
    if (Ar.IsSaving())
    {
        SkeletalMeshName = UAssetManager::Get().GetAssetKeyByObject(EAssetType::SkeletalMesh, PreviewSkeletalMesh);
    }

    Ar << SkeletalMeshName;

    if (Ar.IsLoading())
    {
        if (UObject* Asset = UAssetManager::Get().GetAsset(EAssetType::SkeletalMesh, SkeletalMeshName))
        {
            PreviewSkeletalMesh = Cast<USkeletalMesh>(Asset);
        }
    }

    BodySetups.SerializePtrAsset(Ar);
    ConstraintInstances.SerializePtrAsset(Ar);
}
