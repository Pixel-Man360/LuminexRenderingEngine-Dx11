#pragma once

#include "Transform.h"

namespace Engine::Graphics
{
    class Mesh;

    class RenderObject
    {
    public:
        RenderObject(Mesh* mesh);

        Transform& GetTransform();
        Mesh* GetMesh() const;

    private:
        Mesh* m_mesh = nullptr;
        Transform m_transform;
    };
}
