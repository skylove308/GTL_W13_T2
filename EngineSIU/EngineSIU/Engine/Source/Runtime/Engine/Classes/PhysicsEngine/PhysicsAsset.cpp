
#include "PhysicsAsset.h"

#include "Engine/SkeletalMesh.h"

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
