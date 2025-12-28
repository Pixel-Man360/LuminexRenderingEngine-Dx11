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

void RenderObject::SetTexture(ID3D11ShaderResourceView* texture)
{
    m_texture = texture;
}

ID3D11ShaderResourceView* RenderObject::GetTexture() const
{
    return m_texture;
}