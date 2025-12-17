#include "Transform.h"

using namespace Engine::Graphics;
using namespace DirectX;

Transform::Transform()
    : m_position(0.0f, 0.0f, 0.0f),
	  m_scale(1.0f, 1.0f, 1.0f),
	m_rotation(0.0f, 0.0f, 0.0f, 1.0f) // Identity quaternion
{
}


void Transform::SetPosition(const XMFLOAT3& position)
{
	m_position = position;
}

void Transform::SetScale(const XMFLOAT3& scale)
{
	m_scale = scale;
}

void Transform::SetRotation(const XMFLOAT4& quaternion)
{
	m_rotation = quaternion;
}

void Transform::RotateAxisAngle(const XMFLOAT3& axis, float radians)
{
	XMVECTOR qCurrent = XMLoadFloat4(&m_rotation);
	XMVECTOR qDelta = XMQuaternionRotationAxis(XMLoadFloat3(&axis), radians);
	qCurrent = XMQuaternionMultiply(qDelta, qCurrent);
	qCurrent = XMQuaternionNormalize(qCurrent);

	XMStoreFloat4(&m_rotation, qCurrent);
}


XMFLOAT3 Transform::GetPosition() const
{
	return m_position;
}

XMFLOAT3 Transform::GetScale() const
{
	return m_scale;
}

XMFLOAT4 Transform::GetRotation() const
{
	return m_rotation;
}

XMMATRIX Transform::GetWorldMatrix() const
{
	XMMATRIX S = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&m_rotation));
	XMMATRIX T = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

	return S * R * T;
}
