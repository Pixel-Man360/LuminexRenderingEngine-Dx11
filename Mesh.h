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

        // Create a simple triangle mesh (3 vertices). Returns true on success.
        bool CreateTriangle(ID3D11Device* device);

        // Bind the vertex buffer and issue draw call (no indices)
        void Draw(ID3D11DeviceContext* context);

        void Release();

    private:
        ComPtr<ID3D11Buffer> m_vertexBuffer;
        UINT m_vertexCount = 0;
    };

} // namespace Engine::Graphics
