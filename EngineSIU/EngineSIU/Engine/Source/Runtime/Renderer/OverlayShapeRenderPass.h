#pragma once
#include "IRenderPass.h"
#include "Container/Array.h"
#include "Define.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "Math/Shapes.h"
#include "Math/Color.h"

class FOverlayShapeRenderPass : public IRenderPass
{
public:
    FOverlayShapeRenderPass();
    virtual ~FOverlayShapeRenderPass() override;
    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    virtual void PrepareRenderArr() override;
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void ClearRenderArr() override;

private:
    void CreateShaders();
    void CreateBlendState();
    void CreateBuffers();

    void CreateSphereBuffer(int NumSegments, int NumRings);
    void CreateCapsuleBuffer(int NumSegments, int NumRings);
    void CreateBoxBuffer();
    void CreateConeBuffer(int NumSegments);

    void CreateConstants();

    void StartRender(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void EndRender(const std::shared_ptr<FEditorViewportClient>& Viewport);

    template <typename TConstant>
    void RenderShapeArray(
        const TArray<TConstant>& ShapeConstants,
        const wchar_t* vertexShaderName,
        const wchar_t* pixelShaderName,
        const wchar_t* vertexBufferName,
        const wchar_t* indexBufferName,
        const char* constantBufferName,
        int           shaderRegister,
        int           constantBufferSize
    );


protected:
    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;
    
    TArray< TPair<Shape::FRay, FLinearColor> > Rays;
    TArray< TPair<Shape::FSphere, FLinearColor> > Spheres;
    TArray< TPair<Shape::FBox, FLinearColor> > Boxes;
    TArray< TPair<Shape::FOrientedBox, FLinearColor> > OrientedBoxes;
    TArray< TPair<Shape::FCapsule, FLinearColor> > Capsules;
    TArray< TPair<Shape::FPlane, FLinearColor> > Planes;
    TArray< TPair<Shape::FCone, FLinearColor> > Cones;
    TArray< TPair<Shape::FEllipticalCone, FLinearColor> > EllipticalCones;

    constexpr static int32 ConstantBufferSize = 512;

    ID3D11BlendState* AlphaBlendState;
    ID3D11DepthStencilState* NoZWriteState;
};


template <typename TConstant>
void FOverlayShapeRenderPass::RenderShapeArray(
    const TArray<TConstant>& ShapeConstants,
    const wchar_t* vertexShaderName,
    const wchar_t* pixelShaderName,
    const wchar_t* vertexBufferName,
    const wchar_t* indexBufferName,
    const char* constantBufferName,
    int           shaderRegister,
    int           constantBufferSize
)
{
    if (ShapeConstants.Num() == 0)
        return;

    // Shader setup
    ShaderManager->SetVertexShaderAndInputLayout(vertexShaderName, Graphics->DeviceContext);
    ShaderManager->SetPixelShader(pixelShaderName, Graphics->DeviceContext);

    // Vertex/Index Buffer setup
    const FVertexInfo& VertexInfo = BufferManager->GetVertexBuffer(vertexBufferName);
    UINT Offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &VertexInfo.VertexBuffer, &VertexInfo.Stride, &Offset);
    const FIndexInfo& IndexInfo = BufferManager->GetIndexBuffer(indexBufferName);
    Graphics->DeviceContext->IASetIndexBuffer(IndexInfo.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Constant Buffer binding (vertex/pixel)
    BufferManager->BindConstantBuffer(FString(constantBufferName), shaderRegister, ::EShaderStage::Vertex);
    BufferManager->BindConstantBuffer(FString(constantBufferName), shaderRegister, ::EShaderStage::Pixel);

    // Draw in batches
    int BufferIndex = 0;
    int Total = ShapeConstants.Num();
    while (BufferIndex < Total)
    {
        TArray<TConstant> SubBuffer;
        for (int j = 0; j < constantBufferSize && BufferIndex < Total; ++j, ++BufferIndex)
        {
            SubBuffer.Add(ShapeConstants[BufferIndex]);
        }
        if (SubBuffer.Num() > 0)
        {
            BufferManager->UpdateConstantBuffer(FString(constantBufferName), SubBuffer);
            Graphics->DeviceContext->DrawIndexedInstanced(IndexInfo.NumIndices, SubBuffer.Num(), 0, 0, 0);
        }
    }
}
