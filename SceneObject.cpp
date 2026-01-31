#include "SceneObject.h"

using namespace Engine::Graphics;

uint32_t SceneObject::s_nextID = 1;

SceneObject::SceneObject() : m_id(s_nextID++), m_name("SceneObject")
{
}

SceneObject::SceneObject(const std::string& name) : m_id(s_nextID++), m_name(name)
{
}

const std::string& SceneObject::GetName() const
{
	return m_name;
}

void SceneObject::SetName(const std::string& name)
{
	m_name = name;
}

uint32_t SceneObject::GetID() const
{
	return m_id;
}

Transform& SceneObject::GetTransform()
{
	return m_transform;
}

void SceneObject::SetMesh(Mesh* mesh)
{
	m_mesh = mesh;
}

Mesh* SceneObject::GetMesh() const
{
	return m_mesh;
}

void SceneObject::SetVisible(bool visible)
{
	m_visible = visible;
}

bool SceneObject::IsVisible() const
{
	return m_visible;
}
