#include "PhysicsAssetViewerPanel.h"

#include "SkeletalMeshViewerPanel.h"
#include "Engine/EditorEngine.h"
#include "ReferenceSkeleton.h"
#include "Engine/Classes/Engine/SkeletalMesh.h"
#include "Engine/Classes/Engine/FbxLoader.h"
#include "Engine/Classes/Components/SkeletalMeshComponent.h"
#include "UnrealEd/ImGuiWidget.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Physics/PhysicsManager.h"
#include <PxPhysicsAPI.h>

PhysicsAssetViewerPanel::PhysicsAssetViewerPanel()
{
    SetSupportedWorldTypes(EWorldTypeBitFlag::PhysicsAssetViewer);
}

void PhysicsAssetViewerPanel::Render()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
    {
        return;
    }

    if (BoneIconSRV == nullptr || NonWeightBoneIconSRV == nullptr) {
        LoadBoneIcon();
    }

    /* Pre Setup */
    float PanelWidth = (Width) * 0.2f - 5.0f;
    float PanelHeight = (Height) * 0.9f;

    float PanelPosX = 5.0f;
    float PanelPosY = 5.0f;

    ImVec2 MinSize(140, 100);
    ImVec2 MaxSize(FLT_MAX, 1000);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);
    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    constexpr ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar;

    if (Engine->ActiveWorld && Engine->ActiveWorld->WorldType == EWorldType::PhysicsAssetViewer) {

        if (CopiedRefSkeleton == nullptr) {
            CopyRefSkeleton(); // 선택된 액터/컴포넌트로부터 스켈레톤 정보 복사
        }

        // CopiedRefSkeleton이 여전히 null이면 렌더링하지 않음
        if (CopiedRefSkeleton == nullptr || CopiedRefSkeleton->RawRefBoneInfo.IsEmpty()) {
            ImGui::Begin("Bone Hierarchy", nullptr, PanelFlags); // 창은 표시하되 내용은 비움
            ImGui::Text("No skeleton selected or skeleton has no bones.");
            ImGui::End();
            return;
        }

        ImGui::Begin("Bone Hierarchy", nullptr, PanelFlags); // 창 이름 변경

        // 검색 필터 추가 (선택 사항)
        // static char BoneSearchText[128] = "";
        // ImGui::InputText("Search", BoneSearchText, IM_ARRAYSIZE(BoneSearchText));
        // FString SearchFilter(BoneSearchText);

        // 루트 본부터 시작하여 트리 렌더링
        for (int32 i = 0; i < CopiedRefSkeleton->RawRefBoneInfo.Num(); ++i)
        {
            if (CopiedRefSkeleton->RawRefBoneInfo[i].ParentIndex == INDEX_NONE) // 루트 본인 경우
            {
                // RenderBoneTree 호출 시 Engine 포인터 전달
                RenderBoneTree(*CopiedRefSkeleton, i, Engine /*, SearchFilter */);
            }
        }
        ImGui::End();
    }

    float ExitPanelWidth = (Width) * 0.2f - 6.0f;
    float ExitPanelHeight = 30.0f;

    const float margin = 10.0f;

    float ExitPanelPosX = Width - ExitPanelWidth;
    float ExitPanelPosY = Height - ExitPanelHeight - 10;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::SetNextWindowSize(ImVec2(ExitPanelWidth, ExitPanelHeight), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(ExitPanelPosX, ExitPanelPosY), ImGuiCond_Always);

    constexpr ImGuiWindowFlags ExitPanelFlags =
        ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoBackground
        | ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("Exit Viewer", nullptr, ExitPanelFlags);
    if (ImGui::Button("Exit Viewer", ImVec2(ExitPanelWidth, ExitPanelHeight))) {
        ClearRefSkeletalMeshComponent();
        UEditorEngine* EdEngine = Cast<UEditorEngine>(GEngine);
        EdEngine->EndPhysicsAssetViewer();
    }
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::Separator();
    ImGui::Text("Current Constraints:");
   
    if (RefSkeletalMeshComponent)
    {
        TArray<FConstraintInstance*> Constraints = RefSkeletalMeshComponent->GetSkeletalMeshAsset()->GetPhysicsAsset()->ConstraintInstances;
        for (int32 i = 0; i < Constraints.Num(); ++i)
        {
            ImGui::Text("%s ", *Constraints[i]->JointName);
            ImGui::SameLine();
            if (ImGui::SmallButton(("Remove##" + FString::FromInt(i)).operator*()))
            {
                RemoveConstraint(i);
            }
        }
    }

}

void PhysicsAssetViewerPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}

void PhysicsAssetViewerPanel::SetSkeletalMesh(USkeletalMesh* SMesh)
{
    SkeletalMesh = SMesh;
}

int32 PhysicsAssetViewerPanel::GetSelectedBoneIndex() const
{
    return SelectedBoneIndex;
}

FString PhysicsAssetViewerPanel::GetSelectedBoneName() const
{
    if (SelectedBoneIndex == INDEX_NONE || !SkeletalMesh)
        return TEXT("");
    const auto& RefSkel = SkeletalMesh->GetSkeleton()->GetReferenceSkeleton();
    return RefSkel.RawRefBoneInfo[SelectedBoneIndex].Name.ToString();
}

void PhysicsAssetViewerPanel::ClearRefSkeletalMeshComponent()
{
    if (RefSkeletalMeshComponent)
    {
        RefSkeletalMeshComponent = nullptr;
    }
    if (CopiedRefSkeleton)
    {
        CopiedRefSkeleton = nullptr;
    }
    //if (PrevAnimDataModel)
    //{
    //    PrevAnimDataModel = nullptr;
    //}
}

void PhysicsAssetViewerPanel::AddBody(int32 BoneIndex, const FName& BoneName)
{
    physx::PxVec3 BonePos = physx::PxVec3(CopiedRefSkeleton->RawRefBonePose[BoneIndex].GetTranslation().X, CopiedRefSkeleton->RawRefBonePose[BoneIndex].GetTranslation().Y, CopiedRefSkeleton->RawRefBonePose[BoneIndex].GetTranslation().Z);
    physx::PxVec3 Rotation = physx::PxVec3(CopiedRefSkeleton->RawRefBonePose[BoneIndex].GetRotation().X, CopiedRefSkeleton->RawRefBonePose[BoneIndex].GetRotation().Y, CopiedRefSkeleton->RawRefBonePose[BoneIndex].GetRotation().Z);
    physx::PxVec3 HalfScale = physx::PxVec3(0.5f, 0.5f, 0.5f); // 예시로 0.5로 설정, 실제 스케일은 필요에 따라 조정

    UBodySetup* BodySetup = FObjectFactory::ConstructObject<UBodySetup>(nullptr);

    AggregateGeomAttributes GeomAttributes;
    GeomAttributes.Offset = FVector(BonePos.x, BonePos.y, BonePos.z);
    GeomAttributes.Rotation = FRotator(Rotation.x, Rotation.y, Rotation.z); 
    GeomAttributes.Extent = FVector(HalfScale.x, HalfScale.y, HalfScale.z);

    BodySetup->GeomAttributes.Add(GeomAttributes);
    BodySetup->SetBoneName(BoneName);
    RefSkeletalMeshComponent->GetSkeletalMeshAsset()->GetPhysicsAsset()->BodySetups.Add(BodySetup);
}

void PhysicsAssetViewerPanel::RemoveBody(const FName& BoneName)
{
    TArray<UBodySetup*>& BodySetups = RefSkeletalMeshComponent->GetSkeletalMeshAsset()->GetPhysicsAsset()->BodySetups;  

    for(int i = 0; i < BodySetups.Num(); ++i)
    {
        if (BodySetups[i]->BoneName == BoneName)
        {
            BodySetups.RemoveAt(i);
            break;
        }
    }
}

void PhysicsAssetViewerPanel::AddConstraint(const UBodySetup* Body1, const UBodySetup* Body2)
{
    UPhysicsAsset* PhysicsAsset = RefSkeletalMeshComponent->GetSkeletalMeshAsset()->GetPhysicsAsset();
    FConstraintInstance* NewConstraint = new FConstraintInstance();
 
    NewConstraint->JointName = GetCleanBoneName(Body1->BoneName.ToString()) + ":" + GetCleanBoneName(Body2->BoneName.ToString());
    NewConstraint->ConstraintBone1 = Body1->BoneName.ToString();
    NewConstraint->ConstraintBone2 = Body2->BoneName.ToString();

    if (PhysicsAsset)
    {
        PhysicsAsset->ConstraintInstances.Add(NewConstraint);
    }
}

void PhysicsAssetViewerPanel::RemoveConstraint(int32 ConstraintIndex)
{
    UPhysicsAsset* PhysicsAsset = RefSkeletalMeshComponent->GetSkeletalMeshAsset()->GetPhysicsAsset();
    if (PhysicsAsset && ConstraintIndex >= 0 && ConstraintIndex < PhysicsAsset->ConstraintInstances.Num())
    {
        FConstraintInstance* ConstraintToRemove = PhysicsAsset->ConstraintInstances[ConstraintIndex];
        if (ConstraintToRemove)
        {
            delete ConstraintToRemove;
            PhysicsAsset->ConstraintInstances.RemoveAt(ConstraintIndex);
        }
    }
}
 
void PhysicsAssetViewerPanel::LoadBoneIcon()
{
    BoneIconSRV = FEngineLoop::ResourceManager.GetTexture(L"Assets/Viewer/Bone_16x.PNG")->TextureSRV;
    NonWeightBoneIconSRV = FEngineLoop::ResourceManager.GetTexture(L"Assets/Viewer/BoneNonWeighted_16x.PNG")->TextureSRV;
    BodyInstanceIconSRV = FEngineLoop::ResourceManager.GetTexture(L"Assets/Viewer/GroupActor_16x.PNG")->TextureSRV;
}

void PhysicsAssetViewerPanel::CopyRefSkeleton()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    const FReferenceSkeleton& OrigRef = Engine->PhysicsAssetViewerWorld
        ->GetSkeletalMeshComponent()->GetSkeletalMeshAsset()
        ->GetSkeleton()->GetReferenceSkeleton();

    CopiedRefSkeleton = new FReferenceSkeleton();
    CopiedRefSkeleton->RawRefBoneInfo = OrigRef.RawRefBoneInfo;
    CopiedRefSkeleton->RawRefBonePose = OrigRef.RawRefBonePose;
    CopiedRefSkeleton->InverseBindPoseMatrices = OrigRef.InverseBindPoseMatrices;
    CopiedRefSkeleton->RawNameToIndexMap = OrigRef.RawNameToIndexMap;

    RefSkeletalMeshComponent = Engine->PhysicsAssetViewerWorld->GetSkeletalMeshComponent();
}

void PhysicsAssetViewerPanel::RenderBoneTree(const FReferenceSkeleton& RefSkeleton, int32 BoneIndex, UEditorEngine* Engine /*, const FString& SearchFilter */)
{
    // 0) 본 이름, 부모 본 이름 구하기
    const FMeshBoneInfo& BoneInfo = RefSkeleton.RawRefBoneInfo[BoneIndex];
    const FString& ShortBoneName = GetCleanBoneName(BoneInfo.Name.ToString());

    int32 ParentIndex = RefSkeleton.RawRefBoneInfo[BoneIndex].ParentIndex;
    FString ParentBoneName;
    if (ParentIndex != INDEX_NONE)
    {
        ParentBoneName = GetCleanBoneName(
            RefSkeleton.RawRefBoneInfo[ParentIndex].Name.ToString()
        );
    }

    // ImGui TreeNodeEx 로 본 이름 그리기…
    // 1) ImGui ID 충돌 방지
    ImGui::PushID(BoneIndex);

    // 2) 자식 유무 판별 후 TreeNode 플래그 설정
    ImGuiTreeNodeFlags NodeFlags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

    // 자식이 없는 본은 리프 노드로 처리 (화살표 없음)
    bool bHasChildren = false;
    for (int32 i = 0; i < RefSkeleton.RawRefBoneInfo.Num(); ++i)
    {
        if (RefSkeleton.RawRefBoneInfo[i].ParentIndex == BoneIndex)
        {
            bHasChildren = true;
            break;
        }
    }
    if (!bHasChildren)
    {
        NodeFlags |= ImGuiTreeNodeFlags_Leaf; // 자식 없으면 리프 노드
        NodeFlags &= ~ImGuiTreeNodeFlags_OpenOnArrow; // 리프 노드는 화살표로 열 필요 없음
    }

    ImGui::Image((ImTextureID)BoneIconSRV, ImVec2(16, 16));  // 16×16 픽셀 크기
    ImGui::SameLine();
    // ImGui::TreeNodeEx (본 이름, 플래그)
    // 이름 부분만 클릭 가능하도록 하려면 ImGui::Selectable을 함께 사용하거나 커스텀 로직 필요
    // 여기서는 TreeNodeEx 자체의 클릭 이벤트를 사용
    bool bNodeOpen = ImGui::TreeNodeEx(*ShortBoneName, NodeFlags);

    if (ImGui::BeginPopupContextItem("BonePopup"))
    {
        if (ImGui::MenuItem("Add BodyInstance"))
        {
            AddBody(BoneIndex, BoneInfo.Name);
        }
        ImGui::EndPopup();
    }

    if (bNodeOpen) // 노드가 열려있다면
    {
        UPhysicsAsset* PhysicsAsset = RefSkeletalMeshComponent->GetSkeletalMeshAsset()->GetPhysicsAsset();
        if(PhysicsAsset)
        {
            // 본에 해당하는 Body가 있는지 확인
            for (int32 i = 0; i < PhysicsAsset->BodySetups.Num(); ++i)
            {
                UBodySetup* Body = PhysicsAsset->BodySetups[i];
                if (Body && Body->BoneName == BoneInfo.Name)
                {
                    ImGui::Image((ImTextureID)BodyInstanceIconSRV, ImVec2(16, 16));
                    ImGui::SameLine();
                    ImGui::TreeNodeEx(*ShortBoneName, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                    if (ImGui::BeginPopupContextItem("BodyInstPopup"))
                    {
                        if (ImGui::BeginMenu("Add Constraint"))
                        {
                            // 생성된 ConstraintInstance 리스트를 순회
                            TArray<UBodySetup*>& OppoBody = PhysicsAsset->BodySetups;
                            for (int i = 0; i < OppoBody.Num(); ++i)
                            {
                                char buf[32];
                                sprintf(buf, "%s", *GetCleanBoneName(OppoBody[i]->BoneName.ToString()));

                                if (ImGui::MenuItem(buf))
                                {
                                    // 선택되면 즉시 Constraint 추가
                                    AddConstraint(Body, OppoBody[i]);
                                }
                            }
                            ImGui::EndMenu();
                        }
                        if (ImGui::MenuItem("Remove BodyInstance"))
                        {
                            RemoveBody(BoneInfo.Name);
                        }
                        ImGui::EndPopup();
                    }
                }
            }
        }

        // 자식 본들 재귀적으로 처리
        for (int32 i = 0; i < RefSkeleton.RawRefBoneInfo.Num(); ++i)
        {
            if (RefSkeleton.RawRefBoneInfo[i].ParentIndex == BoneIndex)
            {
                RenderBoneTree(RefSkeleton, i, Engine /*, SearchFilter */); // 재귀 호출 시 Engine 전달
            }
        }

        ImGui::TreePop(); // 트리 노드 닫기
    }
    ImGui::PopID(); // ID 스택 복원
}

// void PhysicsAssetViewerPanel::RenderAnimationSequence(const FReferenceSkeleton& RefSkeleton, UEditorEngine* Engine)

FString PhysicsAssetViewerPanel::GetCleanBoneName(const FString& InFullName)
{
    // 1) 계층 구분자 '|' 뒤 이름만 취하기
    int32 barIdx = InFullName.FindChar(TEXT('|'),
        /*case*/ ESearchCase::CaseSensitive,
        /*dir*/  ESearchDir::FromEnd);
    FString name = (barIdx != INDEX_NONE)
        ? InFullName.RightChop(barIdx + 1)
        : InFullName;

    // 2) 네임스페이스 구분자 ':' 뒤 이름만 취하기
    int32 colonIdx = name.FindChar(TEXT(':'),
        /*case*/ ESearchCase::CaseSensitive,
        /*dir*/  ESearchDir::FromEnd);
    if (colonIdx != INDEX_NONE)
    {
        return name.RightChop(colonIdx + 1);
    }
    return name;
}

//void SkeletalMeshViewerPanel::RenderAnimationPanel(float PanelPosX, float PanelPosY, float PanelWidth, float PanelHeight)


