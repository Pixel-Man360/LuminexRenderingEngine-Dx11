#pragma once

#include <DirectXMath.h>
#include <vector>

namespace Engine::Scene
{
	class Transform
	{

	public:
		Transform();
		~Transform();

		void SetPosition(const DirectX::XMFLOAT3& position);
		void SetScale(const DirectX::XMFLOAT3& scale);
		void SetRotation(const DirectX::XMFLOAT4& quaternion);
		void SetParent(Transform* parent);
		void Translate(const DirectX::XMFLOAT3& delta);
		void Rotate(const DirectX::XMFLOAT3& deltaEuler);
		void RotateAxisAngle(const DirectX::XMFLOAT3& axis, float radians);
		

		DirectX::XMFLOAT3 GetPosition() const;
		DirectX::XMFLOAT3 GetScale() const;
		DirectX::XMFLOAT4 GetRotation() const;

		DirectX::XMMATRIX GetWorldMatrix() const;
		DirectX::XMMATRIX GetLocalMatrix() const;

		
		Transform* GetParent() const;
		const std::vector<Transform*>& GetChildren() const;


	private:
		DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT4 m_rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::XMFLOAT3 m_scale = { 1.0f, 1.0f, 1.0f };
		Transform* m_parent = nullptr;
		std::vector<Transform*> m_children;

		void AddChild(Transform* child);
		void RemoveChild(Transform* child);
	};
};
