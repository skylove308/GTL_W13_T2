#pragma once
#include "OverlayShapeRenderPass.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;

class FGeometryDebugRenderPass : public FOverlayShapeRenderPass
{
    using Super = FOverlayShapeRenderPass;
public:
    FGeometryDebugRenderPass();
    virtual ~FGeometryDebugRenderPass() override;
    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    virtual void PrepareRenderArr() override;
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void ClearRenderArr() override;

private:
    void RenderStaticComp(UStaticMeshComponent* StaticComp, bool bPreviewWorld);
    void RenderSkelComp(USkeletalMeshComponent* SkelComp, bool bPreviewWorld);

    // FOverlayShapeRenderPass을(를) 통해 상속됨
    void PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    void CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    void CreateResource() override;
};
