#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;


namespace Engine::Graphics
{

    struct Vertex
    {
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
        XMFLOAT2 UV;
    };

    class Mesh
    {
    public:
        Mesh() = default;
        ~Mesh() = default;

        bool CreateCube(ID3D11Device* device);
		bool CreatePlane(ID3D11Device* device);

        void Draw(ID3D11DeviceContext* context);

        void Release();

    private:
        ComPtr<ID3D11Buffer> m_vertexBuffer;
        ComPtr<ID3D11Buffer> m_indexBuffer;
        UINT m_indexCount = 0;

    };

} // namespace Engine::Graphics
