
#include "Asset/SkeletalMeshAsset.h"
#include "SkeletalMesh.h"

#include "PhysicsEngine/PhysicsAsset.h"
#include "UObject/ObjectFactory.h"

USkeletalMesh::USkeletalMesh()
{
    /////////////// 테스트 코드
    PhysicsAsset = FObjectFactory::ConstructObject<UPhysicsAsset>(this);
    /////////////////////////
}

void USkeletalMesh::SetRenderData(std::unique_ptr<FSkeletalMeshRenderData> InRenderData)
{
    RenderData = std::move(InRenderData);
}

const FSkeletalMeshRenderData* USkeletalMesh::GetRenderData() const
{
    return RenderData.get(); 
}

void USkeletalMesh::SerializeAsset(FArchive& Ar)
{
    if (Ar.IsLoading())
    {
        if (!RenderData)
        {
            RenderData = std::make_unique<FSkeletalMeshRenderData>();
        }
    }

    RenderData->Serialize(Ar);
}


