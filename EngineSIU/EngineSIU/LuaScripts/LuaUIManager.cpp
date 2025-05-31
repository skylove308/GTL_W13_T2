#include "LuaUIManager.h"
#include "Engine/Source/Runtime/Core/Math/Color.h"
#include "Engine/Engine.h"
#include "Engine/Source/Developer/LuaUtils/LuaTextUI.h"


void LuaUIManager::CreateUI(FName InName)
{


    UpdateUIArrayForSort();
}

void LuaUIManager::CreateText(FName InName, RectTransform InRectTransform, int InSortOrder, FString InText, FName FontStyleName, float InFontSize, FColor InFontColor)
{
    ImFont* FindFont = GetFontStyleByName(FontStyleName);

    LuaTextUI* NewTextUI =  new LuaTextUI(InName, InRectTransform, InText, InSortOrder, FindFont, InFontSize, InFontColor);

    UIMap.Add(InName, NewTextUI);
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
    ImGuiIO& io = ImGui::GetIO();

    ImGuiWindowFlags WindowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBackground | 
        ImGuiWindowFlags_NoInputs;  // 일단 입력 가로채는 문제 있어서 추가함 이후에 수정 필요할지도

    ImGui::SetNextWindowPos(ImVec2(-5, -5), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x + 10, io.DisplaySize.y + 10), ImGuiCond_Always);

    ImGui::Begin("LuaUI", nullptr, WindowFlags);


    for (LuaUI* UI : UIArrayForSort)
    {
        if (UI != nullptr && UI->GetVisible()) 
        {
            UI->DrawImGuiUI();
        }
    }

    ImGui::End();
}

void LuaUIManager::TestCODE()
{
    CreateText("TestTEXT", RectTransform(0, 0, 100, 100, AnchorDirection::MiddleCenter), 10, FString("Chan GOOOD!"), FName("Default"), 30, FColor(1, 0, 0, 1));
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

ImFont* LuaUIManager::GetFontStyleByName(FName FontName)
{
    // 1) Find()를 호출해서 ImFont**(포인터-투-포인터) 얻기
    ImFont** FoundPtr = FontMap.Find(FontName);

    // 2) null 체크 후, *FoundPtr → ImFont* 자체를 돌려준다.
    if (FoundPtr != nullptr)
    {
        return *FoundPtr;  // 맵에 해당 키가 있으면 ImFont*를 반환
    }
    else
    {
        return nullptr;    // 없으면 null 반환
    }
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

    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();

    /** Font load */
    ImFont* AddFont = io.Fonts->Fonts[0];
    
    FontMap.Add(FName("Default"), AddFont);

    TestCODE();
}

void LuaUIManager::UpdateUIArrayForSort()
{
    UIArrayForSort.Empty();

    for (auto& Pair : UIMap) 
    {
        UIArrayForSort.Add(Pair.Value);
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
