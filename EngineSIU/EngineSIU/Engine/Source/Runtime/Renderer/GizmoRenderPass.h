#pragma once

#include "Define.h"
#include "RenderPassBase.h"

class UEditorEngine;
class UGizmoBaseComponent;
class FDXDBufferManager;
class FGraphicsDevice;
class FEditorViewportClient;

class FGizmoRenderPass : public FRenderPassBase
{
public:
    FGizmoRenderPass() = default;
    virtual ~FGizmoRenderPass() override = default;

    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage) override;

    virtual void PrepareRenderArr() override;

    virtual void ClearRenderArr() override;

    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;

protected:
    virtual void PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    
    virtual void CreateResource() override;

    void UpdateShader();
    
    // Gizmo 한 개 렌더링 함수
    void RenderGizmoComponent(UGizmoBaseComponent* GizmoComp, const std::shared_ptr<FEditorViewportClient>& Viewport);

    TArray<UGizmoBaseComponent*> GizmoComponents;

private:
    bool ShouldRenderGizmo(const UEditorEngine* EditorEngine) const;
};
