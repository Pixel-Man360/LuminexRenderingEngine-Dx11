#pragma once
#include "../Scene/Scene.h"
#include "../Scene/SceneObject.h"
#include <DirectXMath.h>

namespace Engine::Editor
{
    class MousePicker
    {
    public:
 
        static Engine::Scene::SceneObject* Pick(
            Engine::Scene::Scene* scene,
            int mouseX, int mouseY,
            const DirectX::XMMATRIX& view,
            const DirectX::XMMATRIX& projection,
            const DirectX::XMFLOAT3& cameraPos,
            int screenWidth, int screenHeight);
        
    private:
  
        static void ScreenToWorldRay(
            int mouseX, int mouseY,
            const DirectX::XMMATRIX& view,
            const DirectX::XMMATRIX& projection,
            int screenWidth, int screenHeight,
            DirectX::XMVECTOR& rayOrigin,
            DirectX::XMVECTOR& rayDirection);
        

        static bool RaySphereIntersect(
            const DirectX::XMVECTOR& rayOrigin,
            const DirectX::XMVECTOR& rayDir,
            const DirectX::XMFLOAT3& sphereCenter,
            float sphereRadius,
            float& distance);
    };
}
