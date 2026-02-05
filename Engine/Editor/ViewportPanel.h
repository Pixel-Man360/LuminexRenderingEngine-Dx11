// ViewportPanel.h
#pragma once
#include "EditorPanel.h"
#include <d3d11.h>
#include <wrl/client.h>

namespace Engine::Graphics { class Renderer; }

namespace Engine::Editor
{
    class ViewportPanel : public EditorPanel
    {
    public:
        void Draw(EditorContext& context) override;
        void SetRenderer(Engine::Graphics::Renderer* renderer) { m_renderer = renderer; }
        
        // Get viewport state for input handling
        bool IsHovered() const { return m_isHovered; }
        bool IsFocused() const { return m_isFocused; }
        
        // Get viewport image bounds (screen coordinates)
        float GetImagePosX() const { return m_imagePosX; }
        float GetImagePosY() const { return m_imagePosY; }
        float GetImageWidth() const { return m_imageWidth; }
        float GetImageHeight() const { return m_imageHeight; }
        
    private:
        Engine::Graphics::Renderer* m_renderer = nullptr;
        bool m_isHovered = false;
        bool m_isFocused = false;
        
        // Image bounds in screen coordinates
        float m_imagePosX = 0;
        float m_imagePosY = 0;
        float m_imageWidth = 0;
        float m_imageHeight = 0;
    };
}
