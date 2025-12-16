#pragma once
#include <DirectXMath.h>

using namespace DirectX;

namespace Engine::Graphics
{
    class Camera
    {
    public:
        Camera();

        void Update(float dt);
        XMMATRIX GetViewMatrix() const;

    private:
        XMFLOAT3 m_position;
        float m_pitch;
        float m_yaw;
        float m_moveSpeed;
        float m_mouseSensitivity;
    };
}
