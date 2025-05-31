#pragma once

#include "Engine/Source/Developer/LuaUtils/LuaUI.h"
#include "Engine/Source/Runtime/CoreUObject/UObject/NameTypes.h"
#include "Engine/Source/Runtime/Core/Container/Map.h"

class LuaUIManager 
{
public:
    static LuaUIManager& Get()
    {
        static LuaUIManager Instance;
        return Instance;
    }


    void CreateUI(FName InName);
    // Text의 경우 크기는 FontSize에만 따라가도록 일단 구현
    void CreateText(FName InName, FString InText, RectTransform InRectTransform, int InSortOrder, float InFontSize, FColor InFontColor);
    void CreateImage(FName InName, FString TexturePath, RectTransform InRectTransform, int InSortOrder);
    void CreateButton(FName InName, FString LuaFunctionName, RectTransform InRectTransform, int InSortOrder);

    
    void DrawLuaUIs();
    void UpdateCanvasRectTransform(HWND hWnd);

    RectTransform GetCanvasRectTransform() { return CanvasRectTransform; }

private:
    LuaUIManager();
    ~LuaUIManager() = default;
    LuaUIManager(const LuaUIManager&) = delete;
    LuaUIManager& operator=(const LuaUIManager&) = delete;

    void UpdateUIArrayForSort();    // Create 하거나 SortOrder 바꿀 때에 호출 필요

    TMap<FName, LuaUI> UIMap;
    TArray<LuaUI*> UIArrayForSort;  // Sort하여 Render하기 위해 Array 사용

    RectTransform CanvasRectTransform;

};
