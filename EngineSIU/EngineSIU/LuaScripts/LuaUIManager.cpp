#include "LuaUIManager.h"
#include "Engine/Source/Runtime/Core/Math/Color.h"
#include "Engine/Engine.h"



void LuaUIManager::CreateUI(FName InName)
{


    UpdateUIArrayForSort();
}

void LuaUIManager::CreateText(FName InName, FString InText, RectTransform InRectTransform, int InSortOrder, float InFontSize, FColor InFontColor)
{

    UpdateUIArrayForSort();
}

void LuaUIManager::CreateImage(FName InName, FString TexturePath, RectTransform InRectTransform, int InSortOrder)
{

    UpdateUIArrayForSort();
}

void LuaUIManager::CreateButton(FName InName, FString LuaFunctionName, RectTransform InRectTransform, int InSortOrder)
{

    UpdateUIArrayForSort();
}

void LuaUIManager::DrawLuaUIs()
{
    for (LuaUI* UI : UIArrayForSort)
    {
        if (UI != nullptr && UI->GetVisible()) 
        {
            UI->DrawImGuiUI();
        }
    }

}

void LuaUIManager::UpdateCanvasRectTransform(HWND hWnd)
{
    // 창의 크기만큼 크기 지정
    // Direction은 왼쪽 상단으로 지정하여 바로 코드에 적용할 수 있도록 함

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    CanvasRectTransform.Size.X = clientRect.right - clientRect.left;
    CanvasRectTransform.Size.Y = clientRect.bottom - clientRect.top;
}

LuaUIManager::LuaUIManager()
{
    CanvasRectTransform.AnchorDir = TopLeft;
    CanvasRectTransform.Position.X = 0.f;
    CanvasRectTransform.Position.Y = 0.f;

    uint32 ClientWidth, ClientHeight;

    GEngineLoop.GetClientSize(ClientWidth, ClientHeight);

    CanvasRectTransform.Size.X = ClientWidth;
    CanvasRectTransform.Size.Y = ClientHeight;
}

void LuaUIManager::UpdateUIArrayForSort()
{
    UIArrayForSort.Empty();

    for (auto& Pair : UIMap) 
    {
        UIArrayForSort.Add(&Pair.Value);
    }

    UIArrayForSort.Sort(
        [](LuaUI* A, LuaUI* B) -> bool
        {
            // SortOrder 값이 낮은 순서대로 그려야 하므로,
            // A가 B보다 낮으면 true를 반환
            return A->GetSortOrder() < B->GetSortOrder();
        }
    );
}
