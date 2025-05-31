#include "OverlayShapeRenderPass.h"
#include "D3D11RHI/GraphicDevice.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealClient.h"
namespace Constants
{
    struct alignas(16) Ray
    {
        FVector Origin;
        float Pad0;
        FVector Direction;
        float Pad1;
        FLinearColor Color;
    };

    struct alignas(16)  Sphere
    {
        FVector Center;
        float Radius;
        FLinearColor Color;
    };

    struct alignas(16) Box
    {
        FVector Min;
        float Pad0;
        FVector Max;
        float Pad1;
        FLinearColor Color;
    };

    struct alignas(16) OrientedBox
    {
        FVector AxisX;
        float ExtentX;
        FVector AxisY;
        float ExtentY;
        FVector AxisZ;
        float ExtentZ;
        FVector Center;
        float Pad0;
        FLinearColor Color;
    };

    struct alignas(16) Capsule
    {
        FVector A; // 캡슐의 한쪽 끝
        float Radius;
        FVector B; // 캡슐의 다른쪽 끝
        float Pad0;
        FLinearColor Color;
    };

    struct alignas(16) Plane
    {
        float X, Y, Z, W; // 평면의 방정식 계수
        FLinearColor Color;
    };

    struct alignas(16) Cone
    {
        FVector Origin;
        float Pad0;
        FVector Direction;
        float Pad1;
        
        float Length;
        float AngleWidth;
        float AngleHeight;
        float Pad2;

        FLinearColor Color;
    };
}

FOverlayShapeRenderPass::FOverlayShapeRenderPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FOverlayShapeRenderPass::~FOverlayShapeRenderPass()
{
}

void FOverlayShapeRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
    CreateShaders();
    CreateBuffers();
    CreateConstants();
    CreateBlendState();
}

void FOverlayShapeRenderPass::PrepareRenderArr()
{
    ClearRenderArr();
    
    // test codes
    //Spheres.Add(TPair<Shape::FSphere, FLinearColor>(Shape::FSphere(FVector(0, 0, 0), 10.f), FLinearColor(1,0,0,0.2)));
    //Spheres.Add(TPair<Shape::FSphere, FLinearColor>(Shape::FSphere(FVector(0, 0, 5), 10.f), FLinearColor(1,0,0,0.2)));
    //Spheres.Add(TPair<Shape::FSphere, FLinearColor>(Shape::FSphere(FVector(0, 0, 10), 10.f), FLinearColor(1,0,0,0.2)));

    //Capsules.Add(TPair<Shape::FCapsule, FLinearColor>(Shape::FCapsule(FVector(0, 0, 0), FVector(10,10,10), 5), FLinearColor(0, 1, 0, 0.2)));
    //Capsules.Add(TPair<Shape::FCapsule, FLinearColor>(Shape::FCapsule(FVector(5, 0, 0), FVector(15,0,0), 3), FLinearColor(0, 1, 0, 0.2)));
    //Capsules.Add(TPair<Shape::FCapsule, FLinearColor>(Shape::FCapsule(FVector(10, 0, 0), FVector(20,0,0), 3), FLinearColor(0, 1, 0, 0.2)));

    //OrientedBoxes.Add(TPair<Shape::FOrientedBox, FLinearColor>(
    //    Shape::FOrientedBox(FVector(1, 1, 0), FVector(-1, 1, 0), FVector(0, 0, 1), FVector(3,3,3), 11,22,33),
    //    FLinearColor(0, 0, 1, 0.2)
    //));
}

void FOverlayShapeRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    StartRender(Viewport);

    // 구 (Sphere)
    {
        TArray<Constants::Sphere> SphereConstants;
        SphereConstants.SetNum(Spheres.Num());
        for (int Index = 0; Index < Spheres.Num(); ++Index)
        {
            const Shape::FSphere Sphere = Spheres[Index].Key;
            const FLinearColor Color = Spheres[Index].Value;
            SphereConstants[Index].Center = Sphere.Center;
            SphereConstants[Index].Radius = Sphere.Radius;
            SphereConstants[Index].Color = Color;
        }
        RenderShapeArray(
            SphereConstants,
            L"OverlayShapeVertexShaderSphere",
            L"OverlayShapePixelShader",
            L"OverlaySphereVertexBuffer",
            L"OverlaySphereIndexBuffer",
            "SphereConstantBuffer",
            11,
            ConstantBufferSize
        );
    }

    // 캡슐 (Capsule)
    {
        TArray<Constants::Capsule> CapsuleConstants;
        CapsuleConstants.SetNum(Capsules.Num());
        for (int Index = 0; Index < Capsules.Num(); ++Index)
        {
            const Shape::FCapsule Capsule = Capsules[Index].Key;
            const FLinearColor Color = Capsules[Index].Value;
            CapsuleConstants[Index].A = Capsule.A;
            CapsuleConstants[Index].B = Capsule.B;
            CapsuleConstants[Index].Radius = Capsule.Radius;
            CapsuleConstants[Index].Color = Color;
        }
        // ... CapsuleConstants 채우기 ...
        RenderShapeArray(
            CapsuleConstants,
            L"OverlayShapeVertexShaderCapsule",
            L"OverlayShapePixelShader",
            L"OverlayCapsuleVertexBuffer",
            L"OverlayCapsuleIndexBuffer",
            "CapsuleConstantBuffer",
            11,
            ConstantBufferSize
        );
    }

    // OrientedBox
    {
        TArray<Constants::OrientedBox> OrientedBoxConstants;
        OrientedBoxConstants.SetNum(OrientedBoxes.Num());
        for (int Index = 0; Index < OrientedBoxes.Num(); ++Index)
        {
            const Shape::FOrientedBox OrientedBox = OrientedBoxes[Index].Key;
            const FLinearColor Color = OrientedBoxes[Index].Value;
            OrientedBoxConstants[Index].AxisX = OrientedBox.AxisX;
            OrientedBoxConstants[Index].ExtentX = OrientedBox.ExtentX;
            OrientedBoxConstants[Index].AxisY = OrientedBox.AxisY;
            OrientedBoxConstants[Index].ExtentY = OrientedBox.ExtentY;
            OrientedBoxConstants[Index].AxisZ = OrientedBox.AxisZ;
            OrientedBoxConstants[Index].ExtentZ = OrientedBox.ExtentZ;
            OrientedBoxConstants[Index].Center = OrientedBox.Center;
            OrientedBoxConstants[Index].Color = Color;
        }
        RenderShapeArray(
            OrientedBoxConstants,
            L"OverlayShapeVertexShaderOrientedBox",
            L"OverlayShapePixelShader",
            L"OverlayBoxVertexBuffer", // Box랑 공유
            L"OverlayBoxIndexBuffer",
            "OrientedBoxConstantBuffer",
            11,
            ConstantBufferSize
        );
    }

    {
        TArray<Constants::Cone> ConeConstants;
        ConeConstants.SetNum(Cones.Num() + EllipticalCones.Num());
        for (int Index = 0; Index < Cones.Num(); ++Index)
        {
            const Shape::FCone Cone = Cones[Index].Key;
            const FLinearColor Color = Cones[Index].Value;
            ConeConstants[Index].Origin = Cone.ApexPosition;
            ConeConstants[Index].Direction = Cone.Direction;
            ConeConstants[Index].Length = Cone.Radius;
            ConeConstants[Index].AngleWidth = Cone.Angle; // 각도는 라디안 단위로 저장
            ConeConstants[Index].AngleHeight = Cone.Angle; // 각도는 라디안 단위로 저장
            ConeConstants[Index].Color = Color;
        }
        for (int Index = 0; Index < EllipticalCones.Num(); ++Index)
        {
            const Shape::FEllipticalCone EllipticalCone = EllipticalCones[Index].Key;
            const FLinearColor Color = EllipticalCones[Index].Value;
            ConeConstants[Cones.Num() + Index].Origin = EllipticalCone.ApexPosition;
            ConeConstants[Cones.Num() + Index].Direction = EllipticalCone.Direction;
            ConeConstants[Cones.Num() + Index].Length = EllipticalCone.Radius;
            ConeConstants[Cones.Num() + Index].AngleWidth = EllipticalCone.AngleWidth; // 각도는 라디안 단위로 저장
            ConeConstants[Cones.Num() + Index].AngleHeight = EllipticalCone.AngleHeight; // 각도는 라디안 단위로 저장
            ConeConstants[Cones.Num() + Index].Color = Color;
        }
        
        RenderShapeArray(
            ConeConstants,
            L"OverlayShapeVertexShaderCone",
            L"OverlayShapePixelShader",
            L"OverlayConeVertexBuffer",
            L"OverlayConeIndexBuffer",
            "ConeConstantBuffer",
            11,
            ConstantBufferSize
        );
    }

    //{
    //    TArray<Constants::Plane> PlaneConstants;
    //    PlaneConstants.SetNum(Planes.Num());
    //    for (int Index = 0; Index < Planes.Num(); ++Index)
    //    {
    //        const Shape::FPlane Plane = Planes[Index].Key;
    //        const FLinearColor Color = Planes[Index].Value;
    //        PlaneConstants[Index].X = Plane.X;
    //        PlaneConstants[Index].Y = Plane.Y;
    //        PlaneConstants[Index].Z = Plane.Z;
    //        PlaneConstants[Index].W = Plane.W;
    //        PlaneConstants[Index].Color = Color;
    //    }
    //    // ... PlaneConstants 채우기 ...
    //    RenderShapeArray(
    //        PlaneConstants,
    //        L"OverlayShapeVertexShaderPlane",
    //        L"OverlayShapePixelShaderPlane",
    //        L"OverlayPlaneVertexBuffer", // Box랑 공유
    //        L"OverlayPlaneIndexBuffer",
    //        "PlaneConstantBuffer",
    //        11,
    //        ConstantBufferSize
    //    );
    //}
    // 박스, 플레인 등도 동일하게 사용 가능

    EndRender(Viewport);
}

void FOverlayShapeRenderPass::ClearRenderArr()
{
    Rays.Empty();
    Spheres.Empty();
    Boxes.Empty();
    OrientedBoxes.Empty();
    Capsules.Empty();
    Planes.Empty();
    Cones.Empty();
    EllipticalCones.Empty();
}

void FOverlayShapeRenderPass::CreateShaders()
{
    D3D11_INPUT_ELEMENT_DESC PositionOnly[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    ShaderManager->AddPixelShader(L"OverlayShapePixelShader", L"Shaders/OverlayShapeShader.hlsl", "DefaultPS", nullptr);

    ShaderManager->AddVertexShaderAndInputLayout(L"OverlayShapeVertexShaderSphere", L"Shaders/OverlayShapeShader.hlsl", "SphereVS", PositionOnly, 1, nullptr);
    //ShaderManager->AddPixelShaderAsync(L"OverlayShapePixelShaderSphere", L"Shaders/OverlayShapeShader.hlsl", "SpherePS", nullptr);

    ShaderManager->AddVertexShaderAndInputLayout(L"OverlayShapeVertexShaderCapsule", L"Shaders/OverlayShapeShader.hlsl", "CapsuleVS", PositionOnly, 1, nullptr);
    //ShaderManager->AddPixelShaderAsync(L"OverlayShapePixelShaderCapsule", L"Shaders/OverlayShapeShader.hlsl", "CapsulePS", nullptr);

    ShaderManager->AddVertexShaderAndInputLayout(L"OverlayShapeVertexShaderOrientedBox", L"Shaders/OverlayShapeShader.hlsl", "OrientedBoxVS", PositionOnly, 1, nullptr);
    //ShaderManager->AddPixelShaderAsync(L"OverlayShapePixelShaderOrientedBox", L"Shaders/OverlayShapeShader.hlsl", "OrientedBoxPS", nullptr);

    ShaderManager->AddVertexShaderAndInputLayout(L"OverlayShapeVertexShaderCone", L"Shaders/OverlayShapeShader.hlsl", "ConeVS", PositionOnly, 1, nullptr);
    //ShaderManager->AddPixelShaderAsync(L"OverlayShapePixelShaderCone", L"Shaders/OverlayShapeShader.hlsl", "ConePS", nullptr);
}

void FOverlayShapeRenderPass::CreateBlendState()
{
    // === [1] Alpha Blend State 생성 ===
    D3D11_BLEND_DESC BlendDesc = {};
    BlendDesc.RenderTarget[0].BlendEnable = TRUE;
    BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    Graphics->Device->CreateBlendState(&BlendDesc, &AlphaBlendState);

    // === [2] DepthStencil State (Z-Write Off) 생성 ===
    D3D11_DEPTH_STENCIL_DESC DepthDesc = {};
    //DepthDesc.DepthEnable = false;  
    DepthDesc.DepthEnable = TRUE;
    DepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    DepthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    Graphics->Device->CreateDepthStencilState(&DepthDesc, &NoZWriteState);
}

void FOverlayShapeRenderPass::CreateBuffers()
{
    CreateSphereBuffer(32, 32);
    CreateCapsuleBuffer(32, 32);
    CreateBoxBuffer();
    CreateConeBuffer(32);
}

void FOverlayShapeRenderPass::CreateSphereBuffer(int NumSegments, int NumRings)
{
    TArray<FVector> Vertices;
    TArray<uint32> Indices;
    // 버텍스 생성
    for (int ring = 0; ring <= NumRings; ++ring)
    {
        float phi = ring * PI / NumRings; // 0 ~ PI
        float y = std::cos(phi); // y축

        for (int seg = 0; seg <= NumSegments; ++seg)
        {
            float theta = seg * 2.0f * PI / NumSegments; // 0 ~ 2PI
            float x = std::sin(phi) * std::cos(theta);
            float z = std::sin(phi) * std::sin(theta);
            Vertices.Emplace(FVector(x, y, z));
        }
    }

    // 인덱스 생성 (삼각형 리스트)
    for (int ring = 0; ring < NumRings; ++ring)
    {
        for (int seg = 0; seg < NumSegments; ++seg)
        {
            int curr = ring * (NumSegments + 1) + seg;
            int next = (ring + 1) * (NumSegments + 1) + seg;

            // 삼각형 1
            Indices.Add(curr);
            Indices.Add(next);
            Indices.Add(curr + 1);

            // 삼각형 2
            Indices.Add(next);
            Indices.Add(next + 1);
            Indices.Add(curr + 1);
        }
    }
    FVertexInfo SphereVertexInfo;
    BufferManager->CreateVertexBuffer<FVector>("OverlaySphereVertexBuffer", Vertices, SphereVertexInfo, D3D11_USAGE_IMMUTABLE, 0);
    FIndexInfo SphereIndexInfo;
    BufferManager->CreateIndexBuffer<uint32>("OverlaySphereIndexBuffer", Indices, SphereIndexInfo, D3D11_USAGE_IMMUTABLE, 0);
}

void FOverlayShapeRenderPass::CreateCapsuleBuffer(int NumSegments, int NumRings)
{
    // radius = 1, halfheight = 0인 캡슐을 생성
    // 구로 보이지만, 특정 vertex의 위치를 조절해서 capsule로 이용
    TArray<FVector> Vertices;
    TArray<uint32> Indices;
    // 1. 상단 반구
    for (int ring = 0; ring <= NumRings / 2; ++ring)
    {
        float phi = ring * PI / NumRings; // 0 ~ PI/2
        float y = FMath::Cos(phi);

        for (int seg = 0; seg <= NumSegments; ++seg)
        {
            float theta = seg * 2.0f * PI / NumSegments;
            float x = FMath::Sin(phi) * FMath::Cos(theta);
            float z = FMath::Sin(phi) * FMath::Sin(theta);
            Vertices.Emplace(FVector(x, y, z));
        }
    }

    int topHemisphereVerts = (NumRings / 2 + 1) * (NumSegments + 1);

    // 2. 실린더
    for (int i = 0; i <= 1; ++i)
    {
        float y = 0;
        for (int seg = 0; seg <= NumSegments; ++seg)
        {
            float theta = seg * 2.0f * PI / NumSegments;
            float x = FMath::Cos(theta);
            float z = FMath::Sin(theta);
            Vertices.Emplace(FVector(x, y, z));
        }
    }

    int cylinderStart = Vertices.Num() - 2 * (NumSegments + 1);

    // 3. 하단 반구
    for (int ring = NumRings / 2; ring <= NumRings; ++ring)
    {
        float phi = ring * PI / NumRings; // PI/2 ~ PI
        float y = FMath::Cos(phi);

        for (int seg = 0; seg <= NumSegments; ++seg)
        {
            float theta = seg * 2.0f * PI / NumSegments;
            float x = FMath::Sin(phi) * FMath::Cos(theta);
            float z = FMath::Sin(phi) * FMath::Sin(theta);
            Vertices.Emplace(FVector(x, y, z));
        }
    }

    int bottomHemisphereStart = Vertices.Num() - (NumRings / 2 + 1) * (NumSegments + 1);

    // 인덱스 생성

    // 1. 상단 반구 삼각형
    for (int ring = 0; ring < NumRings / 2; ++ring)
    {
        for (int seg = 0; seg < NumSegments; ++seg)
        {
            int curr = ring * (NumSegments + 1) + seg;
            int next = (ring + 1) * (NumSegments + 1) + seg;

            Indices.Add(curr);
            Indices.Add(next);
            Indices.Add(curr + 1);

            Indices.Add(next);
            Indices.Add(next + 1);
            Indices.Add(curr + 1);
        }
    }

    // 2. 실린더 옆면
    int topRingStart = (NumRings / 2) * (NumSegments + 1);
    int botRingStart = topRingStart + NumSegments + 1;
    for (int seg = 0; seg < NumSegments; ++seg)
    {
        int top0 = topRingStart + seg;
        int top1 = topRingStart + seg + 1;
        int bot0 = botRingStart + seg;
        int bot1 = botRingStart + seg + 1;

        Indices.Add(top0);
        Indices.Add(bot0);
        Indices.Add(top1);

        Indices.Add(top1);
        Indices.Add(bot0);
        Indices.Add(bot1);
    }

    // 3. 하단 반구 삼각형
    int bottomStart = Vertices.Num() - (NumRings / 2 + 1) * (NumSegments + 1);
    for (int ring = 0; ring < NumRings / 2; ++ring)
    {
        int ringOffset = bottomStart + ring * (NumSegments + 1);
        int nextRingOffset = bottomStart + (ring + 1) * (NumSegments + 1);
        for (int seg = 0; seg < NumSegments; ++seg)
        {
            int curr = ringOffset + seg;
            int next = nextRingOffset + seg;

            Indices.Add(curr);
            Indices.Add(next);
            Indices.Add(curr + 1);

            Indices.Add(next);
            Indices.Add(next + 1);
            Indices.Add(curr + 1);
        }
    }
    FVertexInfo SphereVertexInfo;
    BufferManager->CreateVertexBuffer<FVector>("OverlayCapsuleVertexBuffer", Vertices, SphereVertexInfo, D3D11_USAGE_IMMUTABLE, 0);
    FIndexInfo SphereIndexInfo;
    BufferManager->CreateIndexBuffer<uint32>("OverlayCapsuleIndexBuffer", Indices, SphereIndexInfo, D3D11_USAGE_IMMUTABLE, 0);
}

void FOverlayShapeRenderPass::CreateBoxBuffer()
{
    //TArray<FVector> Vertices;
    TArray<uint32> Indices;

    // 지금 사실 안씀...
    //// 8개의 꼭짓점 (좌표: -1~+1, 중심 기준)
    //Vertices.Add(FVector(-1, -1, -1)); // 0
    //Vertices.Add(FVector(+1, -1, -1)); // 1
    //Vertices.Add(FVector(+1, +1, -1)); // 2
    //Vertices.Add(FVector(-1, +1, -1)); // 3
    //Vertices.Add(FVector(-1, -1, +1)); // 4
    //Vertices.Add(FVector(+1, -1, +1)); // 5
    //Vertices.Add(FVector(+1, +1, +1)); // 6
    //Vertices.Add(FVector(-1, +1, +1)); // 7

    // 12개의 삼각형(2개 per face), 36개의 인덱스
    // 각 면은 시계/반시계 방향으로 구성 (Unreal은 기본적으로 Counter-Clockwise winding)
    uint32 indices[] = {
        // -Z (아래)
        0, 3, 1,
        0, 2, 3,
        // +Z (위)
        4, 5, 7,
        4, 7, 6,
        // -X (왼쪽)
        0, 4, 6,
        0, 6, 2,
        // +X (오른쪽)
        1, 3, 7,
        1, 7, 5,
        // -Y (뒤)
        0, 1, 5,
        0, 5, 4,
        // +Y (앞)
        2, 6, 7,
        2, 7, 3
    };
    for (int i = 0; i < 36; ++i)
    {
        Indices.Add(indices[i]);
    }
    //FVertexInfo VertexInfo;
    //BufferManager->CreateVertexBuffer<FVector>("OverlayBoxVertexBuffer", Vertices, VertexInfo, D3D11_USAGE_IMMUTABLE);
    FIndexInfo IndexInfo;
    BufferManager->CreateIndexBuffer<uint32>("OverlayBoxIndexBuffer", Indices, IndexInfo);
}

// 바닥이 없는 cone
void FOverlayShapeRenderPass::CreateConeBuffer(int NumSegments)
{
    // 버텍스는 내부에서 처리
    TArray<uint32> Indices;

    // 인덱스 생성
    for (int i = 1; i <= NumSegments; ++i)
    {
        int nextIndex = (i % NumSegments) + 1; // 다음 점 인덱스
        Indices.Add(0); // Apex
        Indices.Add(i); // 현재 점
        Indices.Add(nextIndex); // 다음 점
    }
    // 반대 방향으로도 index 추가 (바닥이 없기 때문에 안쪽도 보여야함)
    for (int i = 1; i <= NumSegments; ++i)
    {
        int nextIndex = (i % NumSegments) + 1; // 다음 점 인덱스
        Indices.Add(i); // 현재 점
        Indices.Add(0); // Apex
        Indices.Add(nextIndex); // 다음 점
    }
    //FVertexInfo VertexInfo;
    //BufferManager->CreateVertexBuffer<FVector>("OverlayConeVertexBuffer", Vertices, VertexInfo, D3D11_USAGE_IMMUTABLE, 0);
    FIndexInfo IndexInfo;
    BufferManager->CreateIndexBuffer<uint32>("OverlayConeIndexBuffer", Indices, IndexInfo, D3D11_USAGE_IMMUTABLE, 0);
}

void FOverlayShapeRenderPass::CreateConstants()
{
    BufferManager->CreateBufferGeneric<int>("RayConstantBuffer", nullptr, sizeof(Constants::Ray) * ConstantBufferSize, 
        D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    BufferManager->CreateBufferGeneric<int>("SphereConstantBuffer", nullptr, sizeof(Constants::Sphere) * ConstantBufferSize,
        D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    BufferManager->CreateBufferGeneric<int>("BoxConstantBuffer", nullptr, sizeof(Constants::Box) * ConstantBufferSize,
        D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    BufferManager->CreateBufferGeneric<int>("OrientedBoxConstantBuffer", nullptr, sizeof(Constants::OrientedBox) * ConstantBufferSize,
        D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    BufferManager->CreateBufferGeneric<int>("CapsuleConstantBuffer", nullptr, sizeof(Constants::Capsule) * ConstantBufferSize,
        D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    BufferManager->CreateBufferGeneric<int>("PlaneConstantBuffer", nullptr, sizeof(Constants::Plane) * ConstantBufferSize,
        D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    BufferManager->CreateBufferGeneric<int>("ConeConstantBuffer", nullptr, sizeof(Constants::Cone) * ConstantBufferSize,
        D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
}

void FOverlayShapeRenderPass::StartRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->RSSetViewports(1, &Viewport->GetViewportResource()->GetD3DViewport());

    constexpr EResourceType ResourceType = EResourceType::ERT_PP_ShapeOverlay;
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    const FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(ResourceType);
    const FDepthStencilRHI* DepthStencilRHI = ViewportResource->GetDepthStencil(ResourceType);

    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, DepthStencilRHI->DSV);

    float BlendFactor[4] = { 0, 0, 0, 0 };
    Graphics->DeviceContext->OMSetBlendState(AlphaBlendState, BlendFactor, 0xffffffff);
    Graphics->DeviceContext->OMSetDepthStencilState(NoZWriteState, 0);
}

void FOverlayShapeRenderPass::EndRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    Graphics->DeviceContext->OMSetDepthStencilState(nullptr, 0);
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}
