#pragma once
#include <vector>
#include <memory>

#include "SceneObject.h"

namespace Engine::Scene
{
    class Scene
    {
    public:
        Scene();
        ~Scene();

        SceneObject* CreateObject(const std::string& name = "SceneObject");
        void DestroyObject(SceneObject* object);

        const std::vector<std::unique_ptr<SceneObject>>& GetObjects() const;
		SceneObject* GetSelectedObject() const;

		void SelectObject(SceneObject* object);
		void ClearSelection();

        // Root objects = no parent
        std::vector<SceneObject*> GetRootObjects() const;

		void Update(float deltaTime);

    private:
        std::vector<std::unique_ptr<SceneObject>> m_objects;
		SceneObject* m_selectedObject = nullptr;
    };
}
