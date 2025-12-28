#pragma once

#include "Transform.h"
#include <d3d11.h>

namespace Engine::Graphics
{
    class Mesh;

    class RenderObject
    {
    public:
        RenderObject(Mesh* mesh);

        Transform& GetTransform();
        Mesh* GetMesh() const;
        void SetTexture(ID3D11ShaderResourceView* texture);
        ID3D11ShaderResourceView* GetTexture() const;

    private:
        Mesh* m_mesh = nullptr;
        Transform m_transform;
        ID3D11ShaderResourceView* m_texture = nullptr; 
    };
}
