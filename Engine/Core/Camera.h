#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace Engine::Core
{
    class Camera
    {
    public:
        Camera();

        void Update(float dt);
        XMMATRIX GetViewMatrix() const;

        XMFLOAT3 GetPosition() const { return m_position; }
        void FocusOn(const XMFLOAT3& target, float distance = 8.0f);
    

    private:
        XMVECTOR GetForwardVector() const;
        XMVECTOR GetRightVector() const;
        XMVECTOR GetUpVector() const;

        void ClampPitch();

    private:
        XMFLOAT3 m_position;
        XMFLOAT3 m_pivot;
        XMFLOAT3 m_velocity;

        float m_pitch;
        float m_yaw;
        float m_orbitDistance;

        float m_moveSpeed;
        float m_fastMultiplier;
        float m_mouseSensitivity;
        float m_panSensitivity;
        float m_moveResponsiveness;

        float m_minMoveSpeed;
        float m_maxMoveSpeed;
        float m_minOrbitDistance;
        float m_maxOrbitDistance;
    };
}