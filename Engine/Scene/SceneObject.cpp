#include "SceneObject.h"

using namespace Engine::Scene;
using namespace DirectX;

SceneObject::SceneObject(const std::string& name)
    : m_name(name)
{
}

SceneObject::~SceneObject()
{
    // Detach children safely
    for (SceneObject* child : m_children)
    {
        child->m_parent = nullptr;
    }
    m_children.clear();
}

const std::string& SceneObject::GetName() const
{
    return m_name;
}

void SceneObject::SetName(const std::string& name)
{
    m_name = name;
}

Transform& SceneObject::GetTransform()
{
    return m_transform;
}

const Transform& SceneObject::GetTransform() const
{
    return m_transform;
}

void SceneObject::SetParent(SceneObject* parent)
{
    if (m_parent == parent)
        return;

    // Remove from old parent
    if (m_parent)
    {
        auto& siblings = m_parent->m_children;
        siblings.erase(
            std::remove(siblings.begin(), siblings.end(), this),
            siblings.end()
        );
    }

    m_parent = parent;

    if (m_parent)
    {
        m_parent->m_children.push_back(this);
    }
}

SceneObject* SceneObject::GetParent() const
{
    return m_parent;
}

const std::vector<SceneObject*>& SceneObject::GetChildren() const
{
    return m_children;
}

XMMATRIX SceneObject::GetWorldMatrix() const
{
    XMMATRIX local = m_transform.GetWorldMatrix();

    if (m_parent)
    {
        return local * m_parent->GetWorldMatrix();
    }

    return local;
}


bool SceneObject::IsSelectable() const
{
    return true;
}

void SceneObject::SetSelected(bool selected)
{
	m_selected = selected;
}