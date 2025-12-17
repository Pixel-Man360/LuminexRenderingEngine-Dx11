#include "Camera.h"
#include "Input.h"

using namespace Engine::Graphics;
using namespace Engine::Core;

Camera::Camera()
{
    m_position = { 1.5, 1.5, -5 };
    m_pitch = -0.3f;
    m_yaw = 0.0f;
    m_moveSpeed = 5.0f;
    m_mouseSensitivity = 0.002f;
}

void Camera::Update(float dt)
{
    if (Input::IsMouseButtonDown(VK_RBUTTON))
    {
        float dx = Input::GetMouseDeltaX() * m_mouseSensitivity;
        float dy = Input::GetMouseDeltaY() * m_mouseSensitivity;

        m_yaw += dx;
        m_pitch += dy;

        m_pitch = max(-XM_PIDIV2 + 0.1f, min(XM_PIDIV2 - 0.1f, m_pitch));
    }


    XMVECTOR forward = XMVectorSet
    (
        cosf(m_pitch) * sinf(m_yaw),
        sinf(m_pitch),
        cosf(m_pitch) * cosf(m_yaw),
        0.0f
    );

    XMVECTOR right = XMVector3Cross({ 0,1,0 }, forward);

    XMVECTOR pos = XMLoadFloat3(&m_position);

    if (Input::IsKeyDown('W')) pos += forward * m_moveSpeed * dt;
    if (Input::IsKeyDown('S')) pos -= forward * m_moveSpeed * dt;
    if (Input::IsKeyDown('A')) pos += right * m_moveSpeed * dt;
    if (Input::IsKeyDown('D')) pos -= right * m_moveSpeed * dt;

	if (Input::GetMouseWheelDelta() != 0.0f)
    {
        pos += forward * Input::GetMouseWheelDelta() * m_moveSpeed * 0.5f;
		Input::ResetMouseWheelDelta();
    }

    XMStoreFloat3(&m_position, pos);
}

XMMATRIX Camera::GetViewMatrix() const
{
    XMVECTOR pos = XMLoadFloat3(&m_position);
    XMVECTOR forward = XMVectorSet(
        cosf(m_pitch) * sinf(m_yaw),
        sinf(m_pitch),
        cosf(m_pitch) * cosf(m_yaw),
        0.0f
    );

    return XMMatrixLookToLH(pos, forward, { 0,1,0 });
}
