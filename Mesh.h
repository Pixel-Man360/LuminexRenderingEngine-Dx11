#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace Engine::Graphics
{

    struct SimpleVertex
    {
        float Position[3];
        float Color[4];
    };

    class Mesh
    {
    public:
        Mesh() = default;
        ~Mesh() = default;

        bool CreateTriangle(ID3D11Device* device);
        bool CreateCube(ID3D11Device* device);

        void Draw(ID3D11DeviceContext* context);

        void Release();

    private:
        ComPtr<ID3D11Buffer> m_vertexBuffer;
        ComPtr<ID3D11Buffer> m_indexBuffer;
        UINT m_vertexCount = 0;
        UINT m_indexCount = 0;

    };

} // namespace Engine::Graphics
