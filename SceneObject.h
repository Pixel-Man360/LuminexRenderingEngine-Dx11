#pragma once
#include <string>
#include "Transform.h"

namespace Engine::Graphics
{
	class Mesh;

	class SceneObject
	{
	public:
		SceneObject();
		explicit SceneObject(const std::string& name);

		const std::string& GetName() const;
		void SetName(const std::string& name);

		uint32_t GetID() const;

		Transform& GetTransform();
		const Transform& GetTransform() const;

		void SetMesh(Mesh* mesh);
		Mesh* GetMesh() const;

		void SetVisible(bool visible);
		bool IsVisible() const;



	private:

		uint32_t m_id;
		std::string m_name;
		Transform m_transform;

		Mesh* m_mesh = nullptr;
		bool m_visible = true;

		static uint32_t s_nextID;
	};
}