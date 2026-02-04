#include "StatsPanel.h"
#include <imgui.h>

using namespace Engine::Editor;

void StatsPanel::OnRender()
{
    ImGui::Begin("Stats");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();
}
