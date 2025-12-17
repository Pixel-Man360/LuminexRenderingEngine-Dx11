#include "RenderObject.h"
#include "Mesh.h"

using namespace Engine::Graphics;

RenderObject::RenderObject(Mesh* mesh)
    : m_mesh(mesh)
{
}

Transform& RenderObject::GetTransform()
{
    return m_transform;
}

Mesh* RenderObject::GetMesh() const
{
    return m_mesh;
}
