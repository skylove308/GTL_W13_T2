#pragma once
#include "Engine/EditorEngine.h"
#include "GameFramework/Actor.h"
#include "UnrealEd/EditorPanel.h"

class USkeletalMesh;
class FReferenceSkeleton;
class USkeletalMeshComponent;
struct FBodyInstance;

class PhysicsAssetViewerPanel : public UEditorPanel
{
public:
    PhysicsAssetViewerPanel();

    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

    void SetSkeletalMesh(USkeletalMesh * SMesh);

    int32 GetSelectedBoneIndex() const;
    FString GetSelectedBoneName() const;

    void ClearRefSkeletalMeshComponent();

    void AddBodyInstance();

    void AddConstraint(const FBodyInstance* BodyInstance1, const FBodyInstance* BodyInstance2);
    void RemoveConstraint(int32 ConstraintIndex);

private:
    float Width = 0, Height = 0;
    USkeletalMesh* SkeletalMesh;

    void LoadBoneIcon();
    void CopyRefSkeleton();

    void RenderBoneTree(const FReferenceSkeleton & RefSkeleton, int32 BoneIndex, UEditorEngine * Engine);

    FString GetCleanBoneName(const FString & InFullName);

    ID3D11ShaderResourceView* BoneIconSRV = nullptr;
    ID3D11ShaderResourceView* NonWeightBoneIconSRV = nullptr;

    int32 SelectedBoneIndex = INDEX_NONE;

    FReferenceSkeleton* CopiedRefSkeleton = nullptr;
    USkeletalMeshComponent* RefSkeletalMeshComponent = nullptr;


};
