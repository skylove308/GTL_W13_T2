#pragma once
#include "ReferenceSkeleton.h"
#include "SkinnedAsset.h"
#include "Asset/SkeletalMeshAsset.h"

class UPhysicsAsset;
class USkeleton;

class USkeletalMesh : public USkinnedAsset
{
    DECLARE_CLASS(USkeletalMesh, USkinnedAsset)

public:
    USkeletalMesh();
    virtual ~USkeletalMesh() override = default;

    void SetRenderData(std::unique_ptr<FSkeletalMeshRenderData> InRenderData);

    const FSkeletalMeshRenderData* GetRenderData() const;

    USkeleton* GetSkeleton() const { return Skeleton; }

    void SetSkeleton(USkeleton* InSkeleton);

    virtual void SerializeAsset(FArchive& Ar) override;

    UPhysicsAsset* GetPhysicsAsset() const { return PhysicsAsset; }

    void SetPhysicsAsset(UPhysicsAsset* InPhysicsAsset) { PhysicsAsset = InPhysicsAsset; };
    
    virtual FReferenceSkeleton& GetRefSkeleton();

protected:
    std::unique_ptr<FSkeletalMeshRenderData> RenderData;

    USkeleton* Skeleton = nullptr;

    UPhysicsAsset* PhysicsAsset = nullptr;

    FReferenceSkeleton RefSkeleton;
};
