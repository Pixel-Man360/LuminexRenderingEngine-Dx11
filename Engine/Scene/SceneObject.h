#pragma once
#include <string>
#include <vector>
#include <memory>

#include "Transform.h"

namespace Engine::Scene
{
    class SceneObject
    {
    public:
        SceneObject(const std::string& name = "ScneObject");
        ~SceneObject();

        const std::string& GetName() const;
		void SetName(const std::string& name);


        Transform& GetTransform();
        const Transform& GetTransform() const;

        void SetParent(SceneObject* parent);
        SceneObject* GetParent() const;

        const std::vector<SceneObject*>& GetChildren() const;

        DirectX::XMMATRIX GetWorldMatrix() const;

        bool IsSelectable() const;
		void SetSelected(bool selected);

    private:
        std::string m_name;
        Transform m_transform;

        SceneObject* m_parent = nullptr;
        std::vector<SceneObject*> m_children;

		bool m_selected = false;
    };
}
