#include "HInput.h"
#include "VulkanRenderer.h"
#include "FormMain.h"

void HInput::SetMousePos(glm::vec2 pos)
{
    SDL_WarpMouseGlobal(pos.x, pos.y);
}

void HInput::SetMousePosClient(glm::vec2 pos)
{
    if (VulkanApp::GetFocusForm() && VulkanApp::GetFocusForm()->window)
    {
        SDL_WarpMouseInWindow(VulkanApp::GetFocusForm()->window, pos.x, pos.y);
    }
}

void HInput::ShowCursor(bool bShow)
{
    if (bShow)
        SDL_ShowCursor();
    else
        SDL_HideCursor();
}

glm::vec2 HInput::GetMousePos()
{
    glm::vec2 result(0);
    SDL_GetGlobalMouseState(&result.x, &result.y);
    //return _mousePos;
    return result;
}

glm::vec2 HInput::GetMousePosClient()
{
    glm::vec2 result(0);
    SDL_GetMouseState(&result.x, &result.y);
    //return _mousePosInWindow;
    return result;
}