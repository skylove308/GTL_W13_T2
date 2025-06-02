#pragma once
#include "RenderPassBase.h"
#include "RenderResources.h"

class FSkySphereRenderPass : public FRenderPassBase
{
    // IRenderPass을(를) 통해 상속됨
    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    void PrepareRenderArr() override;
    void ClearRenderArr() override;
    void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    void PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    void CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    void CreateResource() override;
};
