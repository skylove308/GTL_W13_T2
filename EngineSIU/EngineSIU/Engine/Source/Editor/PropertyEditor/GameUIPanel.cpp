#include "GameUIPanel.h"

#include "ImGuiManager.h"
#include "Actors/GameManager.h"
#include "ImGui/imgui.h"
#include "Engine/Engine.h"
#include "Engine/World/World.h"
#include "Engine/Source/Runtime/Engine/Classes/Level.h"

GameUIPanel::GameUIPanel()
{
    SetSupportedWorldTypes(EWorldTypeBitFlag::PIE);
}

void GameUIPanel::Render()
{
    /* Pre Setup */
    float PanelWidth = (Width) * 0.8f;  // 1.0f
    float PanelHeight = (Height) * 0.9f;  // 1.0f

    constexpr float PanelPosX = 0.0f;
    constexpr float PanelPosY = 72.0f;

    constexpr ImVec2 MinSize(300, 72);
    constexpr ImVec2 MaxSize(FLT_MAX, FLT_MAX);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    constexpr ImGuiWindowFlags PanelFlags =
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_MenuBar;
    
    /* Render Start */
    if (!ImGui::Begin("Game UI Panel", nullptr, PanelFlags))
    {
        ImGui::End();
        return;
    }
    auto Actors = GEngine->ActiveWorld->GetActiveLevel()->Actors;
    for (auto Actor : Actors)
    {
        if (Actor->IsA<AGameManager>())
        {
            GameManager = Cast<AGameManager>(Actor);
            break;
        }
    }
    EGameState CurrState = GameManager->GetState();

    switch (CurrState)
    {
    case EGameState::WaitingToStart:
        RenderStartUI();
        break;
    case EGameState::Playing:
        RenderGameUI();
        break;
    case EGameState::GameOver:
        RenderEndUI();
        break;
    }
    ImGui::End();
}

void GameUIPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}

void GameUIPanel::RenderStartUI()
{
    ImVec2 WinPos  = ImVec2(0, 0);
    ImVec2 WinSize = ImVec2(Width, Height);

    std::shared_ptr<FTexture> BGTexPtr = FEngineLoop::ResourceManager.GetTexture(
    L"Assets/Texture/CrossyRoad/StartBG.png");
    ID3D11ShaderResourceView* BGSRV = BGTexPtr ? BGTexPtr->TextureSRV : nullptr;
    if (BGSRV)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 bgTopLeft = ImVec2(0,0);
        ImVec2 bgBottomRight = ImVec2(WinPos.x + WinSize.x, WinPos.y + WinSize.y);
        draw_list->AddImage((ImTextureID)BGSRV, bgTopLeft, bgBottomRight);
    }

    ImGui::Spacing();

    std::shared_ptr<FTexture> PlayTexPtr = FEngineLoop::ResourceManager.GetTexture(
        L"Assets/Texture/CrossyRoad/Play.png"
    );
    std::shared_ptr<FTexture> SettingsTexPtr = FEngineLoop::ResourceManager.GetTexture(
        L"Assets/Texture/CrossyRoad/Settings.png"
    );
    std::shared_ptr<FTexture> ExitTexPtr = FEngineLoop::ResourceManager.GetTexture(
        L"Assets/Texture/CrossyRoad/Exit.png"
    );
    ID3D11ShaderResourceView* PlaySRV     = PlayTexPtr     ? PlayTexPtr->TextureSRV     : nullptr;
    ID3D11ShaderResourceView* SettingsSRV = SettingsTexPtr ? SettingsTexPtr->TextureSRV : nullptr;
    ID3D11ShaderResourceView* ExitSRV     = ExitTexPtr     ? ExitTexPtr->TextureSRV     : nullptr;

    ImVec2 ButtonSize(200,200);

    float StartX = WinPos.x + (WinSize.x * 0.23f);
    float StartY = WinPos.y + WinSize.y * 0.5f;
    float VerticalGap = -40.0f;

    auto DrawImageButton = [&](const char* id, ImTextureID texture, ImVec2 pos, EGameState targetState)
    {
        ImVec2 DrawSize = ButtonSize;

        if (strcmp(id, "##SettingsBtn") == 0)
        {
            DrawSize.x *= 1.2f;
            DrawSize.y *= 1.2f;
            pos.x -= (DrawSize.x - ButtonSize.x) * 0.5f;
            pos.y -= (DrawSize.y - ButtonSize.y) * 0.5f;
        }

        ImGui::SetCursorScreenPos(pos);

        bool Hovered = false;
        ImGui::SetCursorScreenPos(pos);
        Hovered = ImGui::InvisibleButton(id, DrawSize);

        ImGui::SetCursorScreenPos(pos);
        if (ImGui::IsItemHovered())
        {
            float Scale = 1.2f;
            ImVec2 ScaledSize(DrawSize.x * Scale, DrawSize.y * Scale);
            ImVec2 Center = ImVec2(pos.x + DrawSize.x * 0.5f, pos.y + DrawSize.y * 0.5f);
            ImVec2 ScaledPos = ImVec2(Center.x - ScaledSize.x * 0.5f, Center.y - ScaledSize.y * 0.5f);

            ImGui::SetCursorScreenPos(ScaledPos);
            ImGui::Image(texture, ScaledSize);
        }
        else
        {
            ImGui::SetCursorScreenPos(pos);
            ImGui::Image(texture, DrawSize);
        }

        if (Hovered && targetState != EGameState::WaitingToStart)
        {
            GameManager->SetState(targetState);
        }
    };

    if (PlaySRV)
    {
        ImVec2 pos(StartX - (ButtonSize.x * 0.5f), StartY);
        DrawImageButton("##PlayBtn", (ImTextureID)PlaySRV, pos, EGameState::Playing);
    }

    if (SettingsSRV)
    {
        ImVec2 pos(StartX - (ButtonSize.x * 0.5f), StartY + ButtonSize.y + VerticalGap);
        DrawImageButton("##SettingsBtn", (ImTextureID)SettingsSRV, pos, EGameState::None);
    }

    if (ExitSRV)
    {
        ImVec2 pos(StartX - (ButtonSize.x * 0.5f), StartY + 2 * (ButtonSize.y + VerticalGap));
        DrawImageButton("##ExitBtn", (ImTextureID)ExitSRV, pos, EGameState::Exit);
    }
}

void GameUIPanel::RenderGameUI()
{
    int CurrentScore = GameManager->GetScore();
    ImGui::SetCursorScreenPos(ImVec2(30.0f, Height * 0.1f));
    ImGui::PushFont(UImGuiManager::GraffitiFont);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f,  0.1f,  0.0f,  1.0f));
    ImGui::Text("Score : %d", CurrentScore);
    ImGui::PopStyleColor();
    ImGui::PopFont();
}

void GameUIPanel::RenderEndUI()
{
    std::shared_ptr<FTexture> GameOverTex = FEngineLoop::ResourceManager.GetTexture(L"Assets/Texture/CrossyRoad/GameOver.png");
    ID3D11ShaderResourceView* GameOverSRV = GameOverTex ? GameOverTex->TextureSRV : nullptr;

    float OrigW = (float)GameOverTex->Width;
    float OrigH = (float)GameOverTex->Height;
    
    float TargetW = Width * 0.4f;
    float TargetH = OrigH * (TargetW / OrigW);
    
    float ImageStartX = (Width - TargetW) * 0.4f;
    float ImageStartY = Height * 0.1f; 
    ImGui::SetCursorPos(ImVec2(ImageStartX, ImageStartY));

    ImGui::Image((ImTextureID)GameOverSRV, ImVec2(TargetW, TargetH));
    float NextY = ImageStartY + TargetH - 200.0f;

    std::shared_ptr<FTexture> RestartTexPtr = 
        FEngineLoop::ResourceManager.GetTexture(L"Assets/Texture/CrossyRoad/Restart.png");
    std::shared_ptr<FTexture> QuitTexPtr    = 
        FEngineLoop::ResourceManager.GetTexture(L"Assets/Texture/CrossyRoad/Quit.png");

    ID3D11ShaderResourceView* RestartSRV = RestartTexPtr ? RestartTexPtr->TextureSRV : nullptr;
    ID3D11ShaderResourceView* QuitSRV    = QuitTexPtr    ? QuitTexPtr->TextureSRV    : nullptr;

    float BtnOrigW = (float)RestartTexPtr->Width;
    float BtnOrigH = (float)RestartTexPtr->Height;

    float ButtonTargetW = TargetW * 0.4f;
    float ButtonTargetH = BtnOrigH * (ButtonTargetW / BtnOrigW);
    ImVec2 ButtonSize(ButtonTargetW, ButtonTargetH);

    float TotalButtonsWidth = ButtonSize.x * 2;
    float ButtonsStartX = (Width - TotalButtonsWidth) * 0.4f;

    auto DrawImageButton = [&](const char* id, 
                               ImTextureID   texture, 
                               ImVec2        pos, 
                               EGameState    targetState)
    {
        ImGui::SetCursorScreenPos(pos);
        bool HoveredAndClicked = ImGui::InvisibleButton(id, ButtonSize);
        ImGui::SetCursorScreenPos(pos);
        if (ImGui::IsItemHovered())
        {
            float scale = 1.2f;
            ImVec2 scaledSize(ButtonTargetW * scale, ButtonTargetH * scale);
            ImVec2 center = ImVec2(pos.x + ButtonTargetW * 0.5f, pos.y + ButtonTargetH * 0.5f);
            ImVec2 scaledPos = ImVec2(center.x - scaledSize.x * 0.5f, center.y - scaledSize.y * 0.5f);

            ImGui::SetCursorScreenPos(scaledPos);
            ImGui::Image(texture, scaledSize);
        }
        else
        {
            ImGui::SetCursorScreenPos(pos);
            ImGui::Image(texture, ImVec2(ButtonTargetW, ButtonTargetH));
        }

        if (HoveredAndClicked)
        {
            GameManager->SetState(targetState);
        }
    };
    if (RestartSRV)
    {
        ImVec2 PosRestart(ButtonsStartX, NextY);
        DrawImageButton("##RestartBtn", (ImTextureID)RestartSRV, PosRestart, EGameState::Restart);
    }
    if (QuitSRV)
    {
        ImVec2 PosQuit(ButtonsStartX + ButtonSize.x + 20.0f, NextY);
        DrawImageButton("##QuitBtn", (ImTextureID)QuitSRV, PosQuit, EGameState::Exit);
    }
}
