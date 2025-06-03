#include "GameUIPanel.h"

#include "Actors/GameManager.h"
#include "ImGui/imgui.h"
#include "Engine/Engine.h"
#include "Engine/World/World.h"
#include "Engine/Source/Runtime/Engine/Classes/Level.h"
#include "Engine/Classes/Actors/GameManager.h"

GameUIPanel::GameUIPanel()
{
    SetSupportedWorldTypes(EWorldTypeBitFlag::PIE);
}

void GameUIPanel::Render()
{
    /* Pre Setup */
    const ImGuiIO& IO = ImGui::GetIO();
    ImFont* IconFont = IO.Fonts->Fonts[FEATHER_FONT];
    constexpr ImVec2 IconSize = ImVec2(32, 32);

    constexpr float PanelPosX = 0.0f;
    constexpr float PanelPosY = 0.0f;

    constexpr ImVec2 MinSize(300, 72);
    constexpr ImVec2 MaxSize(FLT_MAX, FLT_MAX);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(Width, Height), ImGuiCond_Always);

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
    float verticalGap = -40.0f;

    auto DrawImageButton = [&](const char* id, ImTextureID texture, ImVec2 pos, EGameState targetState)
    {
        ImVec2 drawSize = ButtonSize;

        if (strcmp(id, "##SettingsBtn") == 0)
        {
            drawSize.x *= 1.2f;
            drawSize.y *= 1.2f;
            pos.x -= (drawSize.x - ButtonSize.x) * 0.5f;
            pos.y -= (drawSize.y - ButtonSize.y) * 0.5f;
        }

        ImGui::SetCursorScreenPos(pos);

        bool hovered = false;
        ImGui::SetCursorScreenPos(pos);
        hovered = ImGui::InvisibleButton(id, drawSize);

        ImGui::SetCursorScreenPos(pos);
        if (ImGui::IsItemHovered())
        {
            ImGui::Image(texture, drawSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.5f, 1.2f, 2.0f, 1.0f));
        }
        else
        {
            ImGui::Image(texture, drawSize);
        }

        if (hovered && targetState != EGameState::WaitingToStart)
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
        ImVec2 pos(StartX - (ButtonSize.x * 0.5f), StartY + ButtonSize.y + verticalGap);
        DrawImageButton("##SettingsBtn", (ImTextureID)SettingsSRV, pos, EGameState::None);
    }

    if (ExitSRV)
    {
        ImVec2 pos(StartX - (ButtonSize.x * 0.5f), StartY + 2 * (ButtonSize.y + verticalGap));
        DrawImageButton("##ExitBtn", (ImTextureID)ExitSRV, pos, EGameState::Exit);
    }
}

void GameUIPanel::RenderGameUI()
{
    int currentScore = GameManager->GetScore();
    ImGui::Text("Score: %d", currentScore);
}

void GameUIPanel::RenderEndUI()
{
    std::shared_ptr<FTexture> GameOverTex = FEngineLoop::ResourceManager.GetTexture(L"Assets/Texture/CrossyRoad/Logo.png");
    ID3D11ShaderResourceView* GameOverSRV     = GameOverTex     ? GameOverTex->TextureSRV     : nullptr;
    GameOverSRV = GameOverTex->TextureSRV;
    ImVec2 RestartLogoSize(GameOverTex->Width, GameOverTex->Height);
    ImGui::SetCursorPosX(0.0f);
    if (GameOverSRV)
    {
        ImTextureID texID = (ImTextureID)GameOverSRV;
        ImGui::Image(texID, RestartLogoSize);
    }

    ImGui::Spacing();

    ImVec2 BtnSize(100, 0);
    float SpacingBetween = 20.0f;
    float TotalButtonsWidth = BtnSize.x * 2 + SpacingBetween;
    float ButtonsStartX = (Width - TotalButtonsWidth) * 0.5f;

    ImGui::SetCursorPosX(ButtonsStartX);
    if (ImGui::Button("Restart", BtnSize))
    {
        GameManager->SetState(EGameState::WaitingToStart);
    }

    // ImGui::SameLine(ButtonsStartX + BtnSize.x + SpacingBetween);
    // if (ImGui::Button("Exit", BtnSize))
    // {
    //     GameManager.SetState(EGameState::WaitingToStart);
    // }
}
