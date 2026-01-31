#include "Transform.h"
#include <DirectXMath.h>

using namespace Engine::Graphics;
using namespace DirectX;

Transform::Transform()
    : m_position(0.0f, 0.0f, 0.0f),
	m_rotation(0.0f, 0.0f, 0.0f, 1.0f),
	  m_scale(1.0f, 1.0f, 1.0f),
	m_parent(nullptr)// Identity quaternion
{
}

Transform::~Transform()
{
	if (m_parent)
		m_parent->RemoveChild(this);

	for (auto* child : m_children)
		child->m_parent = nullptr;
}


void Transform::SetPosition(const XMFLOAT3& position)
{
	m_position = position;
}

void Transform::SetRotation(const XMFLOAT4& quaternion)
{
	m_rotation = quaternion;
}


void Transform::SetScale(const XMFLOAT3& scale)
{
	m_scale = scale;
}

void Transform::Translate(const XMFLOAT3& delta)
{
	m_position.x += delta.x;
	m_position.y += delta.y;
	m_position.z += delta.z;
}

void Transform::Rotate(const XMFLOAT3& deltaEuler)
{
	m_rotation.x += deltaEuler.x;
	m_rotation.y += deltaEuler.y;
	m_rotation.z += deltaEuler.z;
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

XMMATRIX Transform::GetLocalMatrix() const
{
	XMMATRIX S = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&m_rotation));
	XMMATRIX T = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

	return S * R * T;
}


XMMATRIX Transform::GetWorldMatrix() const
{
	if (m_parent)
		return GetLocalMatrix() * m_parent->GetWorldMatrix();

	return GetLocalMatrix();
}

void Transform::SetParent(Transform* parent)
{
	if (m_parent == parent)
		return;

	if (m_parent)
		m_parent->RemoveChild(this);

	m_parent = parent;

	if (m_parent)
		m_parent->AddChild(this);
}

Transform* Transform::GetParent() const
{
	return m_parent;
}


const std::vector<Transform*>& Transform::GetChildren() const
{
	return m_children;
}

void Transform::AddChild(Transform* child)
{
	m_children.push_back(child);
}

void Transform::RemoveChild(Transform* child)
{
	auto it = std::find(m_children.begin(), m_children.end(), child);
	if (it != m_children.end())
		m_children.erase(it);
}