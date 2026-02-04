#include "Scene.h"

using namespace Engine::Scene;
using namespace std;

Scene::Scene() {}
Scene::~Scene() {}

SceneObject* Scene::CreateObject(const string& name)
{
	auto obj = make_unique<SceneObject>(name);
	SceneObject* objPtr = obj.get();
	m_objects.emplace_back(std::move(obj));
	return objPtr;
}

void Scene::DestroyObject(SceneObject* object)
{
    if (!object)
        return;


    if (m_selectedObject == object)
        m_selectedObject = nullptr;

    m_objects.erase
    (
        remove_if(m_objects.begin(), m_objects.end(),
        [&](const std::unique_ptr<SceneObject>& o)
        {
            return o.get() == object;
        }),
        m_objects.end()
    );
}

const vector<unique_ptr<SceneObject>>& Scene::GetObjects() const
{
    return m_objects;
}

SceneObject* Scene::GetSelectedObject() const
{
    return m_selectedObject;
}

void Scene::SelectObject(SceneObject* object)
{
    if (m_selectedObject)
        m_selectedObject->SetSelected(false);


    m_selectedObject = object;


    if (m_selectedObject)
        m_selectedObject->SetSelected(true);
}


void Scene::ClearSelection()
{
    if (m_selectedObject)
        m_selectedObject->SetSelected(false);


    m_selectedObject = nullptr;
}

vector<SceneObject*> Scene::GetRootObjects() const
{
    vector<SceneObject*> roots;

    for (auto& obj : m_objects)
    {
        if (obj->GetParent() == nullptr)
            roots.push_back(obj.get());
    }

    return roots;
}


void Scene::Update(float deltaTime)
{
    // Update logic for the scene
}