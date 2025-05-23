#include "DepthOfFieldRenderPass.h"

void FDepthOfFieldRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    FRenderPassBase::Initialize(InBufferManager, InGraphics, InShaderManage);
}

void FDepthOfFieldRenderPass::PrepareRenderArr()
{
    FRenderPassBase::PrepareRenderArr();
}

void FDepthOfFieldRenderPass::ClearRenderArr()
{
    FRenderPassBase::ClearRenderArr();
}

void FDepthOfFieldRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    PrepareRender(Viewport);

    CleanUpRender(Viewport);
}

void FDepthOfFieldRenderPass::PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
}

void FDepthOfFieldRenderPass::CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
}
