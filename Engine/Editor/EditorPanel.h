#pragma once

namespace Engine::Editor
{
    class EditorPanel
    {
    public:
        virtual ~EditorPanel() = default;
        virtual void OnRender() = 0;
    };
}
