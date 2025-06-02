#include "SkySphereRenderPass.h"

void FSkySphereRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    FRenderPassBase::Initialize(InBufferManager, InGraphics, InShaderManager);
}

void FSkySphereRenderPass::PrepareRenderArr()
{
}

void FSkySphereRenderPass::ClearRenderArr()
{
}

void FSkySphereRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
}

void FSkySphereRenderPass::PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
}

void FSkySphereRenderPass::CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
}

void FSkySphereRenderPass::CreateResource()
{
}
