#pragma once

#include "../Scene/Transform.h"
#include <d3d11.h>

namespace Engine::Graphics
{
    class Mesh;

    class RenderObject
    {
    public:
        RenderObject(Mesh* mesh);

        Engine::Scene::Transform& GetTransform();
        Mesh* GetMesh() const;
        void SetTexture(ID3D11ShaderResourceView* texture);
        ID3D11ShaderResourceView* GetTexture() const;

    private:
        Mesh* m_mesh = nullptr;
        Engine::Scene::Transform m_transform;
        ID3D11ShaderResourceView* m_texture = nullptr; 
    };
}
