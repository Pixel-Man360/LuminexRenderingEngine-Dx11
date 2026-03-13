#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;


namespace Engine::Graphics
{

    struct Vertex
    {
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
        XMFLOAT2 TexCoord;
    };

    class Mesh
    {
    public:
        Mesh() = default;
        ~Mesh() = default;

        bool CreateCube(ID3D11Device* device);
		bool CreateSphere(ID3D11Device* device, float radius = 1.0f, uint32_t slices = 32, uint32_t stacks = 16);
		bool CreateCylinder(ID3D11Device* device, float radius = 0.5f, float height = 2.0f, uint32_t slices = 32);
		bool CreateCapsule(ID3D11Device* device, float radius = 0.5f, float height = 2.0f, uint32_t slices = 32, uint32_t stacks = 16);
		bool CreatePlane(ID3D11Device* device);
        bool LoadFromFile(ID3D11Device* device, const std::string& filepath);

        bool CreateFromData(ID3D11Device* device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        void Draw(ID3D11DeviceContext* context);

        void Release();

    private:
        ComPtr<ID3D11Buffer> m_vertexBuffer;
        ComPtr<ID3D11Buffer> m_indexBuffer;
        UINT m_indexCount = 0;

    };

} 
