#include "GeometryDebugRenderPass.h"
#include "Container/Map.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "UnrealEd/EditorViewportClient.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Engine/EditorEngine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Physics/PhysicsManager.h"
#include <Components/ShapeComponent.h>

FGeometryDebugRenderPass::FGeometryDebugRenderPass()  
{  
   // Call the base class constructor properly using the Initialize method or other appropriate logic.  
}

FGeometryDebugRenderPass::~FGeometryDebugRenderPass()
{
    Super::~FOverlayShapeRenderPass();
}

void FGeometryDebugRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    Super::Initialize(InBufferManager, InGraphics, InShaderManager);
}

void FGeometryDebugRenderPass::PrepareRenderArr()
{
    ClearRenderArr();
    Super::PrepareRenderArr();
}

void FGeometryDebugRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    // 그냥 월드일경우
    if (UWorld* World = GEngine->ActiveWorld)
    {
        for (UStaticMeshComponent* SMComp : TObjectRange<UStaticMeshComponent>())
        {
            if (!SMComp || SMComp->GetWorld() != World)
            {
                continue;
            }
            RenderPrimitiveComp(SMComp, false);
        }

        for (USkeletalMeshComponent* SkelComp : TObjectRange<USkeletalMeshComponent>())
        {
            if (!SkelComp || SkelComp->GetWorld() != World)
            {
                continue;
            }
            RenderSkelComp(SkelComp, false);
        }

        for (UShapeComponent* ShapeComp : TObjectRange<UShapeComponent>())
        {
            if (!ShapeComp || ShapeComp->GetWorld() != World)
            {
                continue;
            }
            RenderPrimitiveComp(ShapeComp, false);
        }
    }
    Super::Render(Viewport);

}

void FGeometryDebugRenderPass::ClearRenderArr()
{
    Super::ClearRenderArr();
}

void FGeometryDebugRenderPass::RenderPrimitiveComp(UPrimitiveComponent* PrimitiveComp, bool bPreviewWorld)
{
    UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);
    if (!EditorEngine)
    {
        return;
    }

    TArray<AggregateGeomAttributes>& Geometries = PrimitiveComp->GeomAttributes;
    FLinearColor Color = FLinearColor(1, 0, 1, 0.3);

    if (!bPreviewWorld)
    {
        if (UEditorEngine* Engine = Cast<UEditorEngine>(GEngine))
        {
            if (Engine->GetSelectedActor() == PrimitiveComp->GetOwner())
            {
                Color = FLinearColor(0, 1, 1, 0.3);
            }
        }
    }

    FMatrix InitialMatrix = PrimitiveComp->GetWorldMatrix();
    //InitialMatrix = InitialMatrix.GetMatrixWithoutScale();
    FVector InitialPosition = InitialMatrix.GetTranslationVector();
    FQuat InitialRotation = InitialMatrix.ToQuat();
    InitialRotation.Normalize();

    //FTransform InitialTransform(InitialRotation, InitialPosition);
    FTransform InitialTransform(InitialMatrix);
    for (const AggregateGeomAttributes& Geometry : Geometries)
    {
        if (Geometry.GeomType == EGeomType::EBox)
        {
            FTransform Src = FTransform(Geometry.Rotation, Geometry.Offset);
            FTransform Dst;
            Dst = InitialTransform * Src;
            Shape::FOrientedBox OrientedBox;
            OrientedBox.Center = Dst.GetTranslation();
            OrientedBox.AxisX = Dst.GetRotation().RotateVector(FVector(1, 0, 0));
            OrientedBox.AxisY = Dst.GetRotation().RotateVector(FVector(0, 1, 0));
            OrientedBox.AxisZ = Dst.GetRotation().RotateVector(FVector(0, 0, 1));
            OrientedBox.ExtentX = Geometry.Extent.X;
            OrientedBox.ExtentY = Geometry.Extent.Y;
            OrientedBox.ExtentZ = Geometry.Extent.Z;
            OrientedBoxes.Add(TPair<Shape::FOrientedBox, FLinearColor>(OrientedBox, Color));
        }
        else if (Geometry.GeomType == EGeomType::ESphere)
        {
            FTransform Src = FTransform(Geometry.Offset);
            FTransform Dst;
            Dst = InitialTransform * Src;
            Shape::FSphere Sphere;
            Sphere.Center = Dst.GetTranslation();
            Sphere.Radius = Geometry.Extent.X; // Sphere는 반지름이 동일하므로 X, Y, Z 모두 사용 가능
            Spheres.Add(TPair<Shape::FSphere, FLinearColor>(Sphere, Color));
        }
        else if (Geometry.GeomType == EGeomType::ECapsule)
        {
            FTransform Src = FTransform(Geometry.Rotation, Geometry.Offset);
            FTransform Dst;
            Dst = InitialTransform * Src;
            Shape::FCapsule Capsule;
            Capsule.A = Dst.GetTranslation() + Dst.GetRotation().RotateVector(FVector(0, 0, -Geometry.Extent.Z));
            Capsule.B = Dst.GetTranslation() + Dst.GetRotation().RotateVector(FVector(0, 0, Geometry.Extent.Z));
            Capsule.Radius = Geometry.Extent.X; // 캡슐의 반지름은 X, Y, Z 모두 동일하므로 X 사용
            Capsules.Add(TPair<Shape::FCapsule, FLinearColor>(Capsule, Color));
        }
    }
}

void FGeometryDebugRenderPass::RenderSkelComp(USkeletalMeshComponent* SkelComp, bool bPreviewWorld)
{
    UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);
    if (!EditorEngine)
    {
        return;
    }

    USkeletalMesh* SkeletalMesh = SkelComp->GetSkeletalMeshAsset();
    if (!SkeletalMesh)
    {
        return;
    }

    UPhysicsAsset* PhysicsAsset = SkeletalMesh->GetPhysicsAsset();
    if (!PhysicsAsset)
    {
        return;
    }

    TArray<FBodyInstance*> BodyInstances = SkelComp->GetBodies();
    TArray<UBodySetup*> BodySetups = PhysicsAsset->BodySetups;
   
    // Bone의 정보 얻기
    const FReferenceSkeleton& ReferenceSkeleton = SkeletalMesh->GetRefSkeleton();
    const TArray<FMeshBoneInfo>& RawBoneInfo = ReferenceSkeleton.GetRawRefBoneInfo();
    const TArray<FTransform>& RefBonePoses = SkeletalMesh->GetRefSkeleton().GetRawRefBonePose();

    FLinearColor SphereColor = FLinearColor(1, 0, 1, 0.3);
    FLinearColor BoxColor = FLinearColor(0, 1, 1, 0.3);
    FLinearColor CapsuleColor = FLinearColor(0, 0, 1, 0.3);

    // Rigid Body
    // 선택된 Bone은 다른색으로
    TArray<FMatrix> BoneGlobalMatrices;
    SkelComp->GetCurrentGlobalBoneMatrices(BoneGlobalMatrices);

    for (FBodyInstance* BodyInstance : BodyInstances)
    {
        if (!BodyInstance->BIGameObject || !BodyInstance->BIGameObject->DynamicRigidBody)
        {
            continue; // 해당 BodyInstance가 유효하지 않으면 건너뜀
        }

        int32 BoneIndex = BodyInstance->BoneIndex;
        if (BoneIndex == INDEX_NONE)
        {
            continue; // 해당 Bone이 없으면 건너뜀
        }
        
        physx::PxRigidDynamic* Actor = BodyInstance->BIGameObject->DynamicRigidBody;

        TArray<PxShape*> Shapes;
        int32 ShapeCount = Actor->getNbShapes();
        Shapes.SetNum(ShapeCount);
        Actor->getShapes(Shapes.GetData(), ShapeCount);

        for (physx::PxShape* Shape : Shapes)
        {
            physx::PxTransform ShapeGlobalTransform = Actor->getGlobalPose() * Shape->getLocalPose();
            physx::PxGeometryHolder GeometryHolder = Shape->getGeometry();
            physx::PxGeometryType::Enum GeometryType = GeometryHolder.getType();

            FVector ShapePosition = FVector(ShapeGlobalTransform.p.x, ShapeGlobalTransform.p.y, ShapeGlobalTransform.p.z);
            FQuat ShapeRotation = FQuat(ShapeGlobalTransform.q.x, ShapeGlobalTransform.q.y, ShapeGlobalTransform.q.z, ShapeGlobalTransform.q.w);

            if (GeometryType == physx::PxGeometryType::eSPHERE)
            {
                Shape::FSphere Sphere(ShapePosition, GeometryHolder.sphere().radius);
                Spheres.Add(TPair<Shape::FSphere, FLinearColor>(Sphere, SphereColor));
            }
            else if (GeometryType == physx::PxGeometryType::eBOX)
            {
                Shape::FOrientedBox OrientedBox;
                OrientedBox.Center = ShapePosition;
                OrientedBox.AxisX = ShapeRotation.RotateVector(FVector(1, 0, 0));
                OrientedBox.AxisY = ShapeRotation.RotateVector(FVector(0, 1, 0));
                OrientedBox.AxisZ = ShapeRotation.RotateVector(FVector(0, 0, 1));
                OrientedBox.ExtentX = GeometryHolder.box().halfExtents.x;
                OrientedBox.ExtentY = GeometryHolder.box().halfExtents.y;
                OrientedBox.ExtentZ = GeometryHolder.box().halfExtents.z;

                OrientedBoxes.Add(TPair<Shape::FOrientedBox, FLinearColor>(OrientedBox, BoxColor));

            }
            else if (GeometryType == physx::PxGeometryType::eCAPSULE)
            {
                Shape::FCapsule Capsule;
                Capsule.A = ShapePosition + ShapeRotation.RotateVector(FVector(0, 0, -GeometryHolder.capsule().halfHeight));
                Capsule.B = ShapePosition + ShapeRotation.RotateVector(FVector(0, 0, GeometryHolder.capsule().halfHeight));
                Capsule.Radius = GeometryHolder.capsule().radius;
                Capsules.Add(TPair<Shape::FCapsule, FLinearColor>(Capsule, CapsuleColor));
            }
        }
    }
}

void FGeometryDebugRenderPass::RenderShapeComp(UShapeComponent* ShapeComp)
{
    if (!ShapeComp || !ShapeComp->BodyInstance)
    {
        return;
    }
    FBodyInstance* BodyInstance = ShapeComp->BodyInstance;
    if (!BodyInstance->BIGameObject || !BodyInstance->BIGameObject->DynamicRigidBody)
    {
        return;
    }
    physx::PxRigidDynamic* Actor = BodyInstance->BIGameObject->DynamicRigidBody;
    int32 ShapeCount = Actor->getNbShapes();
    if (ShapeCount <= 0)
    {
        return; // Shape가 없는 경우
    }
    TArray<physx::PxShape*> Shapes;
    Shapes.SetNum(ShapeCount);
    BodyInstance->BIGameObject->DynamicRigidBody->getShapes(Shapes.GetData(), ShapeCount);
    FLinearColor Color = FLinearColor(1, 0, 1, 0.3);
    for (physx::PxShape* Shape : Shapes)
    {
        physx::PxTransform ShapeGlobalTransform = Actor->getGlobalPose() * Shape->getLocalPose();
        physx::PxGeometryHolder GeometryHolder = Shape->getGeometry();
        physx::PxGeometryType::Enum GeometryType = GeometryHolder.getType();
        switch (GeometryType)
        {
        case physx::PxGeometryType::eSPHERE:
        {
            Shape::FSphere Sphere(FVector(ShapeGlobalTransform.p.x, ShapeGlobalTransform.p.y, ShapeGlobalTransform.p.z), GeometryHolder.sphere().radius);
            Spheres.Add(TPair<Shape::FSphere, FLinearColor>(Sphere, Color));
            break;
        }
        case physx::PxGeometryType::eBOX:
        {
            Shape::FOrientedBox OrientedBox;
            OrientedBox.Center = FVector(ShapeGlobalTransform.p.x, ShapeGlobalTransform.p.y, ShapeGlobalTransform.p.z);
            OrientedBox.AxisX = FQuat(ShapeGlobalTransform.q.x, ShapeGlobalTransform.q.y, ShapeGlobalTransform.q.z, ShapeGlobalTransform.q.w).RotateVector(FVector(1, 0, 0));
            OrientedBox.AxisY = FQuat(ShapeGlobalTransform.q.x, ShapeGlobalTransform.q.y, ShapeGlobalTransform.q.z, ShapeGlobalTransform.q.w).RotateVector(FVector(0, 1, 0));
            OrientedBox.AxisZ = FQuat(ShapeGlobalTransform.q.x, ShapeGlobalTransform.q.y, ShapeGlobalTransform.q.z, ShapeGlobalTransform.q.w).RotateVector(FVector(0, 0, 1));
            OrientedBox.ExtentX = GeometryHolder.box().halfExtents.x;
            OrientedBox.ExtentY = GeometryHolder.box().halfExtents.y;
            OrientedBox.ExtentZ = GeometryHolder.box().halfExtents.z;
            OrientedBoxes.Add(TPair<Shape::FOrientedBox, FLinearColor>(OrientedBox, Color));
            break;
        }
        case physx::PxGeometryType::eCAPSULE:
        {
            Shape::FCapsule Capsule;
            Capsule.A = FVector(ShapeGlobalTransform.p.x, ShapeGlobalTransform.p.y, ShapeGlobalTransform.p.z) +
                FQuat(ShapeGlobalTransform.q.x, ShapeGlobalTransform.q.y, ShapeGlobalTransform.q.z, ShapeGlobalTransform.q.w).RotateVector(FVector(0, 0, -GeometryHolder.capsule().halfHeight));
            Capsule.B = FVector(ShapeGlobalTransform.p.x, ShapeGlobalTransform.p.y, ShapeGlobalTransform.p.z) +
                FQuat(ShapeGlobalTransform.q.x, ShapeGlobalTransform.q.y, ShapeGlobalTransform.q.z, ShapeGlobalTransform.q.w).RotateVector(FVector(0, 0, GeometryHolder.capsule().halfHeight));
            Capsule.Radius = GeometryHolder.capsule().radius;
            Capsules.Add(TPair<Shape::FCapsule, FLinearColor>(Capsule, Color));
            break;
        }
        }



    }
}

void FGeometryDebugRenderPass::PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{

}

void FGeometryDebugRenderPass::CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{

}

void FGeometryDebugRenderPass::CreateResource()
{

}
