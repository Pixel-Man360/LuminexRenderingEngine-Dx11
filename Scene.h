#pragma once
#include <vector>
#include <memory>

namespace Engine::Graphics
{
	class SceneObject;

	class Scene
	{
	public:
		Scene();
		~Scene();

		SceneObject* CreateObject(const char* name = "SceneObject");
		void DestroyObject(SceneObject* object);

		void Clear();

		const std::vector<std::unique_ptr<SceneObject>>& GetObjects() const;
		SceneObject* FindByID(uint32_t id) const;

	private:
		std::vector<std::unique_ptr<SceneObject>> m_objects;
	};
}