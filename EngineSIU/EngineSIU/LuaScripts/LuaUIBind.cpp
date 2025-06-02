#include "LuaUIBind.h"
#include "Engine/Source/Runtime/Core/Math/Color.h"
#include "Engine/Classes/Lua/LuaUtils/LuaBindUtils.h"
#include "Engine/Source/Developer/LuaUtils/LuaTextUI.h"
#include "Engine/Source/Developer/LuaUtils/LuaImageUI.h"
#include "Engine/Source/Developer/LuaUtils/LuaSliderUI.h"

void LuaUIBind::Bind(sol::table& Table)
{
    // AnchorDirection enum 등록
    Table.new_enum<AnchorDirection>("AnchorDirection",
        {
            { "TopLeft", AnchorDirection::TopLeft },
            { "TopCenter", AnchorDirection::TopCenter },
            { "TopRight", AnchorDirection::TopRight },
            { "MiddleLeft", AnchorDirection::MiddleLeft },
            { "MiddleCenter", AnchorDirection::MiddleCenter },
            { "MiddleRight", AnchorDirection::MiddleRight },
            { "BottomLeft", AnchorDirection::BottomLeft },
            { "BottomCenter", AnchorDirection::BottomCenter },
            { "BottomRight", AnchorDirection::BottomRight }
        } 
    );

    // RectTransform 바인딩
    Table.Lua_NewUserType(
        RectTransform,

        // 생성자
        sol::constructors<RectTransform(), RectTransform(float, float, float, float, AnchorDirection)>(),

        // 멤버 변수
        LUA_BIND_MEMBER(&RectTransform::Position),
        LUA_BIND_MEMBER(&RectTransform::Size),
        LUA_BIND_MEMBER(&RectTransform::AnchorDir)
    );

    // LuaTextUI 바인딩
    Table.Lua_NewUserType(
        LuaTextUI,

        // 생성자 실질적으로는 사용하지 않음
        // LuaBindUI의 Create 함수를 사용하므로
        sol::constructors<LuaTextUI(FName), LuaTextUI(FName, RectTransform, FString&, int, ImFont*, float, FLinearColor)>(),

        // 멤버 변수
        LUA_BIND_MEMBER(&LuaTextUI::Text),
        LUA_BIND_MEMBER(&LuaTextUI::FontSytle),
        LUA_BIND_MEMBER(&LuaTextUI::FontSize),
        LUA_BIND_MEMBER(&LuaTextUI::FontColor),

        // 멤버 함수
        LUA_BIND_MEMBER(&LuaTextUI::SetText),
        LUA_BIND_MEMBER(&LuaTextUI::SetFont),
        LUA_BIND_MEMBER(&LuaTextUI::SetFontSize),
        LUA_BIND_MEMBER(&LuaTextUI::SetFontColor),
        LUA_BIND_MEMBER(&LuaTextUI::GetNameStr)
    );

    // LuaImageUI 바인딩
    Table.Lua_NewUserType(
        LuaImageUI,

        // 생성자 실질적으로는 사용하지 않음
        // LuaBindUI의 Create 함수를 사용하므로
        sol::constructors<LuaImageUI(FName), LuaImageUI(FName, RectTransform, int, FTexture*, FLinearColor&)>(),

        // 멤버 변수
        // 텍스쳐는 SetTextureByName으로만 바꿀 수 있게함
        LUA_BIND_MEMBER(&LuaImageUI::Color),

        // 멤버 함수
        LUA_BIND_MEMBER(&LuaImageUI::SetTextureByName),
        LUA_BIND_MEMBER(&LuaImageUI::SetColor),
        LUA_BIND_MEMBER(&LuaImageUI::GetNameStr)
    );


    // LuaButtonUI 바인딩


    // LuaSliderUI 바인딩

    Table.Lua_NewUserType(
        LuaSliderUI,

        // 생성자 실질적으로는 사용하지 않음
        // LuaBindUI의 Create 함수를 사용하므로
        sol::constructors<LuaSliderUI(FName)>(),

        // 멤버 변수
        // 텍스쳐는 SetTextureByName으로만 바꿀 수 있게함
        LUA_BIND_MEMBER(&LuaSliderUI::CurrentValue),

        // 멤버 함수
        LUA_BIND_MEMBER(&LuaSliderUI::SetValue)
    );

    // LuaUI 호출 Static Class 바인딩
    Table.Lua_NewUserType(
        LuaUIBind,

        // 생성자
        sol::constructors<LuaUIBind()>(),

        // Static 함수
        LUA_BIND_STATIC(&LuaUIBind::CreateText),
        LUA_BIND_STATIC(&LuaUIBind::CreateImage),
        LUA_BIND_STATIC(&LuaUIBind::CreateButton),
        LUA_BIND_STATIC(&LuaUIBind::CreateSlider),
        LUA_BIND_STATIC(&LuaUIBind::DeleteUI),
        LUA_BIND_STATIC(&LuaUIBind::ClearLuaUI),

        LUA_BIND_STATIC(&LuaUIBind::GetTextUI),
        LUA_BIND_STATIC(&LuaUIBind::GetImageUI),
        LUA_BIND_STATIC(&LuaUIBind::GetSliderUI)
    );
}

void LuaUIBind::CreateText(FString InName, RectTransform InRectTransform, int InSortOrder, FString InText, FString FontStyleName, float InFontSize, FLinearColor InFontColor)
{
    // Lua 쪽에는 FName 있는 것이 큰 의미가 없을 것 같아서 
    // FString을 받아서 FName으로 바꾸도록 작업

    LuaUIManager::Get().CreateText(FName(InName), InRectTransform, InSortOrder, InText, FName(FontStyleName), InFontSize, InFontColor);
}

void LuaUIBind::CreateImage(FString InName, RectTransform InRectTransform, int InSortOrder, FString TextureName, FLinearColor InTextureColor)
{
    LuaUIManager::Get().CreateImage(FName(InName), InRectTransform, InSortOrder, FName(TextureName), InTextureColor);
}

void LuaUIBind::CreateButton(FString InName, RectTransform InRectTransform, int InSortOrder, FString LuaFunctionName)
{
    LuaUIManager::Get().CreateButton(InName, InRectTransform, InSortOrder, LuaFunctionName);
}

void LuaUIBind::CreateSlider(FString InName, RectTransform InRectTransform, int InSortOrder, FString InBackgroundTexture, FLinearColor InBackgroundColor, FString InFillTexture, FLinearColor InFillColor, float InMarginTop, float InMarginBottom, float InMarginLeft, float InMarginRight)
{
    LuaUIManager::Get().CreateSlider(InName, InRectTransform, InSortOrder, InBackgroundTexture, InBackgroundColor, InFillTexture, InFillColor, InMarginTop, InMarginBottom, InMarginLeft, InMarginRight);
}

void LuaUIBind::DeleteUI(FString InName)
{
    LuaUIManager::Get().DeleteUI(InName);
}

LuaTextUI* LuaUIBind::GetTextUI(FString FindName)
{
    return LuaUIManager::Get().GetTextUI(FindName);
}

LuaImageUI* LuaUIBind::GetImageUI(FString FindName)
{
    return LuaUIManager::Get().GetImageUI(FindName);
}

LuaButtonUI* LuaUIBind::GetButtonUI(FString FindName)
{
    return LuaUIManager::Get().GetButtonUI(FindName);
}

LuaSliderUI* LuaUIBind::GetSliderUI(FString FindName)
{
    return LuaUIManager::Get().GetSliderUI(FindName);
}

void LuaUIBind::ClearLuaUI()
{
    LuaUIManager::Get().ClearLuaUI();
}
