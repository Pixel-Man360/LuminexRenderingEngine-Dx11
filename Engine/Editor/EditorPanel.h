#pragma once
#include "EditorContext.h"

namespace Engine::Editor
{
    class EditorPanel
    {
    public:
        virtual ~EditorPanel() = default;
        virtual void Draw(EditorContext& context) = 0;
    };
}
