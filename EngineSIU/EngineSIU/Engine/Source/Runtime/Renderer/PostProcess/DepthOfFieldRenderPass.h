#pragma once

#include "RenderPassBase.h"

struct ID3D11SamplerState;

class FDepthOfFieldRenderPass : public FRenderPassBase
{
public:
    FDepthOfFieldRenderPass() = default;
    virtual ~FDepthOfFieldRenderPass() override = default;

    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage) override;
    virtual void PrepareRenderArr() override;
    virtual void ClearRenderArr() override;
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;

protected:
    virtual void PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport) override;

    virtual void CreateResource() override;

    void PrepareDownSample(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void CleanUpDownSample(const std::shared_ptr<FEditorViewportClient>& Viewport);
    
    void PrepareHorizontalBlur(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void CleanUpHorizontalBlur(const std::shared_ptr<FEditorViewportClient>& Viewport);
    
    void PrepareVerticalBlur(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void CleanUpVerticalBlur(const std::shared_ptr<FEditorViewportClient>& Viewport);
};
