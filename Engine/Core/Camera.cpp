#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Camera.h"
#include "Input.h"

#include <algorithm>
#include <cmath>

using namespace Engine::Core;
using namespace DirectX;

Camera::Camera()
{
    m_position = { 3.0f, 5.0f, -25.0f };
    m_pitch = -0.3f;
    m_yaw = 0.0f;

    m_moveSpeed = 8.0f;
    m_fastMultiplier = 4.0f;
    m_mouseSensitivity = 0.0025f;
    m_panSensitivity = 0.0020f;
    m_moveResponsiveness = 14.0f;

    m_minMoveSpeed = 0.25f;
    m_maxMoveSpeed = 200.0f;

    m_orbitDistance = 10.0f;
    m_minOrbitDistance = 0.1f;
    m_maxOrbitDistance = 1000.0f;

    m_velocity = { 0.0f, 0.0f, 0.0f };

    XMVECTOR position = XMLoadFloat3(&m_position);
    XMVECTOR pivot = position + GetForwardVector() * m_orbitDistance;
    XMStoreFloat3(&m_pivot, pivot);
}

XMVECTOR Camera::GetForwardVector() const
{
    XMVECTOR forward = XMVectorSet(cosf(m_pitch) * sinf(m_yaw), sinf(m_pitch), cosf(m_pitch) * cosf(m_yaw), 0.0f);
    return XMVector3Normalize(forward);
}

XMVECTOR Camera::GetRightVector() const
{
    XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    return XMVector3Normalize(XMVector3Cross(worldUp, GetForwardVector()));
}

XMVECTOR Camera::GetUpVector() const
{
    return XMVector3Normalize(XMVector3Cross(GetForwardVector(), GetRightVector()));
}

void Camera::ClampPitch()
{
    constexpr float pitchLimit = XM_PIDIV2 - 0.01f;
    m_pitch = std::clamp(m_pitch, -pitchLimit, pitchLimit);
}

void Camera::Update(float dt)
{
    dt = std::clamp(dt, 0.0f, 0.05f);

    float mouseX = Input::GetMouseDeltaX();
    float mouseY = Input::GetMouseDeltaY();
    float wheel = Input::GetMouseWheelDelta();

    bool leftMouse = Input::IsMouseButtonDown(VK_LBUTTON);
    bool middleMouse = Input::IsMouseButtonDown(VK_MBUTTON);
    bool rightMouse = Input::IsMouseButtonDown(VK_RBUTTON);

    bool altDown = Input::IsKeyDown(VK_MENU);
    bool shiftDown = Input::IsKeyDown(VK_SHIFT);

    XMVECTOR position = XMLoadFloat3(&m_position);
    XMVECTOR pivot = XMLoadFloat3(&m_pivot);
    XMVECTOR velocity = XMLoadFloat3(&m_velocity);

    if (altDown && leftMouse)
    {
        m_yaw += mouseX * m_mouseSensitivity;
        m_pitch -= mouseY * m_mouseSensitivity;
        ClampPitch();

        position = pivot - GetForwardVector() * m_orbitDistance;
        velocity = XMVectorZero();
    }
    else if (altDown && rightMouse)
    {
        float zoomFactor = std::exp(mouseY * 0.01f);
        m_orbitDistance *= zoomFactor;
        m_orbitDistance = std::clamp(m_orbitDistance, m_minOrbitDistance, m_maxOrbitDistance);

        position = pivot - GetForwardVector() * m_orbitDistance;
        velocity = XMVectorZero();
    }
    else if (middleMouse)
    {
        XMVECTOR right = GetRightVector();
        XMVECTOR up = GetUpVector();

        const float panScale = std::max(0.001f, m_orbitDistance * m_panSensitivity);
        XMVECTOR offset = (-right * mouseX * panScale) + (up * mouseY * panScale);

        position += offset;
        pivot += offset;
        velocity = XMVectorZero();
    }
    else if (rightMouse)
    {
        m_yaw += mouseX * m_mouseSensitivity;
        m_pitch -= mouseY * m_mouseSensitivity;
        ClampPitch();

        if (wheel != 0.0f)
        {
            m_moveSpeed *= std::pow(1.25f, wheel);
            m_moveSpeed = std::clamp(m_moveSpeed, m_minMoveSpeed, m_maxMoveSpeed);
            wheel = 0.0f;
        }

        XMVECTOR forward = GetForwardVector();
        XMVECTOR right = GetRightVector();
        XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMVECTOR moveDirection = XMVectorZero();

        if (Input::IsKeyDown('W')) moveDirection += forward;
        if (Input::IsKeyDown('S')) moveDirection -= forward;
        if (Input::IsKeyDown('D')) moveDirection += right;
        if (Input::IsKeyDown('A')) moveDirection -= right;
        if (Input::IsKeyDown('Q')) moveDirection -= worldUp;
        if (Input::IsKeyDown('E')) moveDirection += worldUp;

        XMVECTOR targetVelocity = XMVectorZero();
        float moveLengthSq = XMVectorGetX(XMVector3LengthSq(moveDirection));

        if (moveLengthSq > 0.0001f)
        {
            moveDirection = XMVector3Normalize(moveDirection);

            float speed = shiftDown ? m_moveSpeed * m_fastMultiplier : m_moveSpeed;
            targetVelocity = moveDirection * speed;
        }

        float smoothing = 1.0f - std::exp(-m_moveResponsiveness * dt);
        velocity = XMVectorLerp(velocity, targetVelocity, smoothing);

        position += velocity * dt;
        pivot = position + forward * m_orbitDistance;
    }
    else
    {
        float smoothing = 1.0f - std::exp(-m_moveResponsiveness * dt);
        velocity = XMVectorLerp(velocity, XMVectorZero(), smoothing);
    }

    if (!rightMouse && wheel != 0.0f)
    {
        float zoomFactor = std::pow(0.85f, wheel);

        m_orbitDistance *= zoomFactor;
        m_orbitDistance = std::clamp(m_orbitDistance, m_minOrbitDistance, m_maxOrbitDistance);

        position = pivot - GetForwardVector() * m_orbitDistance;
    }

    if (Input::GetMouseWheelDelta() != 0.0f) Input::ResetMouseWheelDelta();

    XMStoreFloat3(&m_position, position);
    XMStoreFloat3(&m_pivot, pivot);
    XMStoreFloat3(&m_velocity, velocity);
}

void Camera::FocusOn(const XMFLOAT3& target, float distance)
{
    m_pivot = target;
    m_orbitDistance = std::clamp(distance, m_minOrbitDistance, m_maxOrbitDistance);

    XMVECTOR targetPosition = XMLoadFloat3(&target);
    XMVECTOR position = targetPosition - GetForwardVector() * m_orbitDistance;

    XMStoreFloat3(&m_position, position);
    m_velocity = { 0.0f, 0.0f, 0.0f };
}

XMMATRIX Camera::GetViewMatrix() const
{
    XMVECTOR position = XMLoadFloat3(&m_position);
    return XMMatrixLookToLH(position, GetForwardVector(), GetUpVector());
}