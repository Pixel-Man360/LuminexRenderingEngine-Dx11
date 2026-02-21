#pragma once

#include "../Scene/Scene.h"
#include "../Scene/SceneObject.h"
#include "GizmoMode.h"

namespace Engine::Editor
{
    struct EditorContext
    {
        Engine::Scene::Scene* ActiveScene = nullptr;
        Engine::Scene::SceneObject* SelectedObject = nullptr;
        bool ShowEditor = true;
       
        GizmoMode CurrentGizmoMode = GizmoMode::Translate;
        
        // Scale mode: true = uniform (all axes), false = per-axis
        bool UniformScale = true;
    };
}
