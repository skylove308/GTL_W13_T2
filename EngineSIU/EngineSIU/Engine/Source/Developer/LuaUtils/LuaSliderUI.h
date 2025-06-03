#pragma once

#include "LuaUI.h"
#include "Engine/Source/Runtime/Core/Math/Color.h"

struct FTexture;

class LuaSliderUI : public LuaUI
{
public:
    LuaSliderUI(FName InName);
    LuaSliderUI(FName InName, RectTransform InRectTransform, int InSortOrder
        , FTexture* InBackgroundTexture, FLinearColor& InBackgroundColor
        , FTexture* InFillTexture, FLinearColor& InFillColor
        , float InMarginTop, float InMarginBottom, float InMarginLeft, float InMarginRight);

    virtual void DrawImGuiUI() override;

    // 슬라이더 값을 0.0 ~ 1.0 사이로 설정
    void SetValue(float InValue);

public:
    FTexture* BackgroundTexture;
    FLinearColor BackgroundColor;

    FTexture* FillTexture;
    FLinearColor FillColor;

    float MarginTop;
    float MarginBottom;
    float MarginLeft;
    float MarginRight;

    // 0.0 ~ 1.0 범위
    float CurrentValue;


};
