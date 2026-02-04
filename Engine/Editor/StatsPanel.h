#pragma once
#include "EditorPanel.h"

namespace Engine::Editor
{
    class StatsPanel : public EditorPanel
    {
    public:
        void Draw(EditorContext& context) override;
    };
}
