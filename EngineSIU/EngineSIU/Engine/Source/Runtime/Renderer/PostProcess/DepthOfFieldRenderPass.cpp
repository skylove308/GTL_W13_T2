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

    /**
     * TODO: DOF 순서
     *       1. 전체 해상도 뎁스 맵을 기준으로 CoC(Circle of Confusion) 계산
     *       2. 씬 렌더 결과(Fog 적용 이후가 좋음)를 절반 크기로 다운 샘플링
     *       3. CoC값을 기반으로 가변 크기 커널 블러 적용
     *       4. 커널 블러 결과를 다시 2배 크기로 업 샘플링
     *       5. 합성
     */

    CleanUpRender(Viewport);
}

void FDepthOfFieldRenderPass::PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
}

void FDepthOfFieldRenderPass::CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
}
