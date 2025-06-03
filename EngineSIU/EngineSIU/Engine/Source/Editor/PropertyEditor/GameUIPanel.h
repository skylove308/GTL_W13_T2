#pragma once
#include "D3D11RHI/DXDShaderManager.h"
#include "sol/sol.hpp"
#include "UnrealEd/EditorPanel.h"

class AGameManager;

class GameUIPanel : public UEditorPanel
{
public:
    GameUIPanel();
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

    void RenderStartUI();
    void RenderGameUI();
    void RenderEndUI();

private:
    float Width = 0, Height = 0;
    AGameManager* GameManager = nullptr;
};
