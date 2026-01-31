#include "Scene.h"
#include "SceneObject.h"

using namespace Engine::Graphics;

Scene::Scene() = default;
Scene::~Scene() { Clear(); }

SceneObject* Scene::CreateObject(const char* name)
{
	auto object = std::make_unique<SceneObject>(name);
	SceneObject* objectPtr = object.get();

	m_objects.push_back(std::move(object));
	return objectPtr;
}


void Scene::DestroyObject(SceneObject* object)
{
	if (!object) return;

	m_objects.erase
	(
		std::remove_if
		(
			m_objects.begin(),
			m_objects.end(),
			[object](const std::unique_ptr<SceneObject>& objPtr)
			{
				return objPtr.get() == object;
			}
		),
		m_objects.end()
	);
}


void Scene::Clear()
{
	m_objects.clear();
}

const std::vector<std::unique_ptr<SceneObject>>& Scene::GetObjects() const
{
	return m_objects;
}

SceneObject* Scene::FindByID(uint32_t id) const
{
	for (const auto& object : m_objects)
	{
		if (object->GetID() == id)
		{
			return object.get();
		}
	}
	return nullptr;
}