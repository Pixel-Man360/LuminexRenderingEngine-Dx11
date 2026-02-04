#pragma once
#include "EditorPanel.h"

namespace Engine::Editor
{
    class StatsPanel : public EditorPanel
    {
    public:
        void OnRender() override;
    };
}
