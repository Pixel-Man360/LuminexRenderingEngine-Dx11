#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace Engine::Graphics
{
	class Transform
	{

	public:
		Transform();

		void SetPosition(const XMFLOAT3& position);
		void SetScale(const XMFLOAT3& scale);
		void SetRotation(const XMFLOAT4& quaternion);
		void RotateAxisAngle(const XMFLOAT3& axis, float radians);

		XMFLOAT3 GetPosition() const;
		XMFLOAT3 GetScale() const;
		XMFLOAT4 GetRotation() const;

		XMMATRIX GetWorldMatrix() const;


	private:
		XMFLOAT3 m_position;
		XMFLOAT3 m_scale;
		XMFLOAT4 m_rotation;
	};
};

