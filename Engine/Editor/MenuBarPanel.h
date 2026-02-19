#pragma once
#include "EditorPanel.h"

namespace Engine::Graphics 
{ 
    class Mesh; 
    class Renderer;
}

namespace Engine::Editor
{
    class MenuBarPanel : public EditorPanel
    {
    public:
        void Draw(EditorContext& context) override;
        void SetRenderer(Engine::Graphics::Renderer* renderer) { m_renderer = renderer; }

    private:
        Engine::Graphics::Renderer* m_renderer = nullptr;
    };
}
