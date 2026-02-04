// ViewportPanel.cpp
#include "ViewportPanel.h"
#include "imgui.h"

using namespace Engine::Editor;

void ViewportPanel::Draw(EditorContext&)
{
    ImGui::Begin("Viewport");
    ImGui::Text("Renderer output will go here");
    ImGui::End();
}
