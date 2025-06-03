#include "LuaSliderUI.h"
#include "LuaScripts/LuaUIManager.h"
#include "Engine/Classes/Engine/Texture.h"
#include "Engine/Source/ThirdParty/ImGui/include/ImGui/imgui.h"

LuaSliderUI::LuaSliderUI(FName InName)
    :LuaUI(InName)
{
}

LuaSliderUI::LuaSliderUI(FName InName, RectTransform InRectTransform, int InSortOrder, FTexture* InBackgroundTexture, FLinearColor& InBackgroundColor, FTexture* InFillTexture, FLinearColor& InFillColor, float InMarginTop, float InMarginBottom, float InMarginLeft, float InMarginRight)
    : LuaUI(InName)
    , BackgroundTexture(InBackgroundTexture)
    , BackgroundColor(InBackgroundColor)
    , FillTexture(InFillTexture)
    , FillColor(InFillColor)
    , MarginTop(InMarginTop)
    , MarginBottom(InMarginBottom)
    , MarginLeft(InMarginLeft)
    , MarginRight(InMarginRight)
    , CurrentValue(0.f)
{
    Rect = InRectTransform;
}

void LuaSliderUI::DrawImGuiUI()
{
    if (!GetVisible())
        return;

    // 1) 월드 좌표계에서 절대 위치와 크기 계산
    RectTransform worldRect = GetWorldRectTransform();
    ImVec2 screenPos = ImVec2(worldRect.Position.X, worldRect.Position.Y);
    ImVec2 fullSize = ImVec2(worldRect.Size.X, worldRect.Size.Y);

    // 2) 배경 텍스처가 있으면 그리기
    if (BackgroundTexture != nullptr)
    {
        // 색상 변환 (0.0~1.0)
        ImVec4 bgTint = ImVec4(
            BackgroundColor.R,
            BackgroundColor.G,
            BackgroundColor.B,
            BackgroundColor.A
        );

        ImTextureID bgID = (ImTextureID)(intptr_t)(BackgroundTexture->TextureSRV);
        ImGui::SetCursorScreenPos(screenPos);
        ImGui::Image(
            bgID,
            fullSize,
            ImVec2(0.0f, 0.0f), // UV 전체
            ImVec2(1.0f, 1.0f),
            bgTint
        );
    }

    // 3) 채우기 영역 계산 (마진 적용 후 내부 크기)
    float innerX = screenPos.x + MarginLeft;
    float innerY = screenPos.y + MarginTop;
    float innerWidth = fullSize.x - (MarginLeft + MarginRight);
    float innerHeight = fullSize.y - (MarginTop + MarginBottom);

    if (innerWidth < 0.0f)  innerWidth = 0.0f;
    if (innerHeight < 0.0f) innerHeight = 0.0f;

    // 4) 값을 기준으로 채움 너비 결정 (가로 슬라이더 기준)
    float fillWidth = innerWidth * CurrentValue;
    float fillHeight = innerHeight;

    // 5) 채우기 텍스처 그리기
    if (FillTexture != nullptr && fillWidth > 0.0f && fillHeight > 0.0f)
    {
        ImVec4 fillTint = ImVec4(
            FillColor.R,
            FillColor.G,
            FillColor.B,
            FillColor.A
        );

        ImTextureID fillID = (ImTextureID)(intptr_t)(FillTexture->TextureSRV);

        // 채우기 사각형 위치
        ImVec2 fillPos = ImVec2(innerX, innerY);
        ImVec2 fillSize = ImVec2(fillWidth, fillHeight);

        ImGui::SetCursorScreenPos(fillPos);
        ImGui::Image(
            fillID,
            fillSize,
            ImVec2(0.0f, 0.0f),
            ImVec2(fillWidth / (float)FillTexture->Width,
                fillHeight / (float)FillTexture->Height),
            fillTint
        );
    }
}

void LuaSliderUI::SetValue(float InValue)
{
    CurrentValue = FMath::Clamp(InValue, 0.0f, 1.0f);
}
