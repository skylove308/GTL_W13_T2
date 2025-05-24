#include "DepthOfFieldRenderPass.h"

#include "RendererHelpers.h"
#include "ShaderConstants.h"
#include "UnrealClient.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "Engine/Engine.h"
#include "UnrealEd/EditorViewportClient.h"
#include "World/World.h"

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
    PrepareDownSample(Viewport);
    Graphics->DeviceContext->Draw(6, 0);
    CleanUpDownSample(Viewport);

    PrepareHorizontalBlur(Viewport);
    Graphics->DeviceContext->Draw(6, 0);
    CleanUpHorizontalBlur(Viewport);

    /*
    PrepareVerticalBlur(Viewport);
    Graphics->DeviceContext->Draw(6, 0);
    CleanUpVerticalBlur(Viewport);
    */

    PrepareComposite(Viewport);
    Graphics->DeviceContext->Draw(6, 0);
    CleanUpComposite(Viewport);

    CleanUpRender(Viewport);
}

void FDepthOfFieldRenderPass::PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport) {}
void FDepthOfFieldRenderPass::CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport) {}

void FDepthOfFieldRenderPass::CreateResource()
{
    HRESULT hr = hr = ShaderManager->AddPixelShader(L"DownSamplePixelShader", L"Shaders/DownSampleShader.hlsl", "main");
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to Compile DownSamplePixelShader", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    //hr = ShaderManager->AddPixelShader(L"HorizontalBlurPixelShader", L"Shaders/GaussianBlurShader.hlsl", "main");
    hr = ShaderManager->AddPixelShader(L"HorizontalBlurPixelShader", L"Shaders/BokehDOF.hlsl", "main");
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to Compile HorizontalBlurPixelShader", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    hr = ShaderManager->AddPixelShader(L"DOFComposite", L"Shaders/BokehDOF.hlsl", "main_Composite");
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"Failed to Compile DOFComposite", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    uint32 DepthOfFieldConstantSize = sizeof(FDepthOfFieldConstant);
    hr = BufferManager->CreateBufferGeneric<FDepthOfFieldConstant>("FDepthOfFieldConstant", nullptr, DepthOfFieldConstantSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
}

void FDepthOfFieldRenderPass::PrepareDownSample(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    if (!ViewportResource)
    {
        return;
    }
    
    FRenderTargetRHI* RenderTargetRHI_Scene = ViewportResource->GetRenderTarget(EResourceType::ERT_Scene);
    FRenderTargetRHI* RenderTargetRHI_DownSample2x = ViewportResource->GetRenderTarget(EResourceType::ERT_DownSample2x, 2);

    const FRect ViewportRect = Viewport->GetViewport()->GetRect();
    const float DownSampledWidth = static_cast<float>(FMath::FloorToInt(ViewportRect.Width / 2));
    const float DownSampledHeight = static_cast<float>(FMath::FloorToInt(ViewportRect.Height / 2));

    D3D11_VIEWPORT Viewport_DownSample2x;
    Viewport_DownSample2x.Width = DownSampledWidth;
    Viewport_DownSample2x.Height = DownSampledHeight;
    Viewport_DownSample2x.MinDepth = 0.0f;
    Viewport_DownSample2x.MaxDepth = 1.0f;
    Viewport_DownSample2x.TopLeftX = 0.f;
    Viewport_DownSample2x.TopLeftY = 0.f;
    Graphics->DeviceContext->RSSetViewports(1, &Viewport_DownSample2x);
    
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI_DownSample2x->RTV, nullptr);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, &RenderTargetRHI_Scene->SRV);

    ID3D11VertexShader* VertexShader = ShaderManager->GetVertexShaderByKey(L"FullScreenQuadVertexShader");
    ID3D11PixelShader* PixelShader = ShaderManager->GetPixelShaderByKey(L"DownSamplePixelShader");
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(nullptr);

    Graphics->DeviceContext->PSSetSamplers(0, 1, &Graphics->SamplerState_LinearClamp);
}

void FDepthOfFieldRenderPass::CleanUpDownSample(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, NullSRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_SceneDepth), 1, NullSRV);
}

void FDepthOfFieldRenderPass::PrepareHorizontalBlur(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    if (!ViewportResource)
    {
        return;
    }

    FRenderTargetRHI* RenderTargetRHI_DownSample2x = ViewportResource->GetRenderTarget(EResourceType::ERT_DownSample2x, 2);
    FRenderTargetRHI* RenderTargetRHI_Blur = ViewportResource->GetRenderTarget(EResourceType::ERT_Blur, 2);
    
    FDepthStencilRHI* DepthStencilRHI_Scene = ViewportResource->GetDepthStencil(EResourceType::ERT_Scene);
    
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI_Blur->RTV, nullptr);

    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, &RenderTargetRHI_DownSample2x->SRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_SceneDepth), 1, &DepthStencilRHI_Scene->SRV);
    
    ID3D11VertexShader* VertexShader = ShaderManager->GetVertexShaderByKey(L"FullScreenQuadVertexShader");
    ID3D11PixelShader* PixelShader = ShaderManager->GetPixelShaderByKey(L"HorizontalBlurPixelShader");
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(nullptr);

    Graphics->DeviceContext->PSSetSamplers(10, 1, &Graphics->SamplerState_LinearClamp);

    BufferManager->BindConstantBuffer("FViewportSize", 0, EShaderStage::Pixel);
    BufferManager->BindConstantBuffer("FDepthOfFieldConstant", 1, EShaderStage::Pixel);

    const FRect ViewportRect = Viewport->GetViewport()->GetRect();
    FViewportSize TextureSize = {};
    TextureSize.ViewportSize.X = static_cast<float>(FMath::FloorToInt(ViewportRect.Width / 2));
    TextureSize.ViewportSize.Y = static_cast<float>(FMath::FloorToInt(ViewportRect.Height / 2));
    
    BufferManager->UpdateConstantBuffer("FViewportSize", TextureSize);

    FDepthOfFieldConstant DepthOfFieldConstant;
    if (GEngine->ActiveWorld->WorldType == EWorldType::PIE)
    {
        if (const APlayerController* PC = GEngine->ActiveWorld->GetPlayerController())
        {
            if (const APlayerCameraManager* PCM = PC->PlayerCameraManager)
            {
                DepthOfFieldConstant.F_Stop = PCM->F_Stop;
                DepthOfFieldConstant.SensorWidth = PCM->SensorWidth;
                DepthOfFieldConstant.FocalDistance = PCM->FocalDistance;
                DepthOfFieldConstant.FocalLength = PCM->GetFocalLength();
            }
        }
    }
    else
    {
        DepthOfFieldConstant.F_Stop = Viewport->F_Stop;
        DepthOfFieldConstant.SensorWidth = Viewport->SensorWidth;
        DepthOfFieldConstant.FocalDistance = Viewport->FocalDistance;
        DepthOfFieldConstant.FocalLength = Viewport->GetFocalLength();
    }

    BufferManager->UpdateConstantBuffer("FDepthOfFieldConstant", DepthOfFieldConstant);
}

void FDepthOfFieldRenderPass::CleanUpHorizontalBlur(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, NullSRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_SceneDepth), 1, NullSRV);
}

void FDepthOfFieldRenderPass::PrepareVerticalBlur(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
}

void FDepthOfFieldRenderPass::CleanUpVerticalBlur(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
}

void FDepthOfFieldRenderPass::PrepareComposite(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    if (!ViewportResource)
    {
        return;
    }

    D3D11_VIEWPORT D3DViewport;
    D3DViewport.Width = Viewport->GetD3DViewport().Width;
    D3DViewport.Height = Viewport->GetD3DViewport().Height;
    D3DViewport.MinDepth = 0.0f;
    D3DViewport.MaxDepth = 1.0f;
    D3DViewport.TopLeftX = 0.f;
    D3DViewport.TopLeftY = 0.f;
    Graphics->DeviceContext->RSSetViewports(1, &D3DViewport);
    
    FRenderTargetRHI* RenderTargetRHI_PostProcess = ViewportResource->GetRenderTarget(EResourceType::ERT_PostProcessCompositing);
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI_PostProcess->RTV, nullptr);
    
    FRenderTargetRHI* RenderTargetRHI_Blur = ViewportResource->GetRenderTarget(EResourceType::ERT_Blur, 2);
    FRenderTargetRHI* RenderTargetRHI_Scene = ViewportResource->GetRenderTarget(EResourceType::ERT_Scene);
    FDepthStencilRHI* DepthStencilRHI_Scene = ViewportResource->GetDepthStencil(EResourceType::ERT_Scene);

    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_PostProcess), 1, &RenderTargetRHI_Blur->SRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, &RenderTargetRHI_Scene->SRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_SceneDepth), 1, &DepthStencilRHI_Scene->SRV);
    
    ID3D11VertexShader* VertexShader = ShaderManager->GetVertexShaderByKey(L"FullScreenQuadVertexShader");
    ID3D11PixelShader* PixelShader = ShaderManager->GetPixelShaderByKey(L"DOFComposite");
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(nullptr);

    Graphics->DeviceContext->PSSetSamplers(10, 1, &Graphics->SamplerState_LinearClamp);

    BufferManager->BindConstantBuffer("FViewportSize", 0, EShaderStage::Pixel);
    BufferManager->BindConstantBuffer("FDepthOfFieldConstant", 1, EShaderStage::Pixel);
}

void FDepthOfFieldRenderPass::CleanUpComposite(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, NullSRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_SceneDepth), 1, NullSRV);
}
