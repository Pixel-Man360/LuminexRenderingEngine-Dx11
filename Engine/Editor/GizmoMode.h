#pragma once

namespace Engine::Editor
{
    enum class GizmoMode
    {
        None = 0,
        Translate,
        Rotate,
        Scale
    };
    
    enum class GizmoAxis
    {
        None = 0,
        X,
        Y,
        Z,
        XY,
        XZ,
        YZ,
        All  // For uniform scaling
    };
}
