#pragma once
#include <DirectXMath.h>

namespace Engine::Scene
{
    enum class LightType
    {
        Directional = 0,
        Point = 1
    };

    class LightComponent
    {
    public:
        LightComponent() = default;
        ~LightComponent() = default;

        // Getters
        LightType GetType() const { return m_type; }
        DirectX::XMFLOAT3 GetColor() const { return m_color; }
        float GetIntensity() const { return m_intensity; }
        float GetRange() const { return m_range; }
        bool IsEnabled() const { return m_enabled; }

        // Setters
        void SetType(LightType type) { m_type = type; }
        void SetColor(const DirectX::XMFLOAT3& color) { m_color = color; }
        void SetIntensity(float intensity) { m_intensity = intensity; }
        void SetRange(float range) { m_range = range; }
        void SetEnabled(bool enabled) { m_enabled = enabled; }

    private:
        LightType m_type = LightType::Point;
        DirectX::XMFLOAT3 m_color = { 1.0f, 1.0f, 1.0f };
        float m_intensity = 1.0f;
        float m_range = 10.0f;      
        bool m_enabled = true;
    };
}
