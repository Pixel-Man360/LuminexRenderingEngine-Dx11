#pragma once

#include <DirectXMath.h>
#include <vector>
#include "DeviceResources.h"
#include "Camera.h"
#include "RenderObject.h"


using namespace DirectX;
using namespace std;

namespace Engine::Graphics
{

    class Shader;
    class Mesh;
    class ConstantBuffer;

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        bool Initialize(DeviceResources* deviceResources);
        void Render();
        void SetClearColor(float r, float g, float b, float a);
        void Release();

    private:
        DeviceResources* m_deviceResources = nullptr;
        Shader* m_shader = nullptr;
        Mesh* m_mesh = nullptr;
        ConstantBuffer* m_cbPerObject = nullptr;
        ConstantBuffer* m_cbLight = nullptr;


        // Pipeline states
		ID3D11RasterizerState* m_rasterizerState = nullptr;
		ID3D11DepthStencilState* m_depthStencilState = nullptr;

        // Texture
        ID3D11ShaderResourceView* m_diffuseTexture = nullptr;
        ID3D11SamplerState* m_samplerState = nullptr;

        Camera m_camera;

		vector<RenderObject*> m_renderObjects;


        XMFLOAT4 m_clearColor{ 0.1f, 0.5f, 0.6f, 1.0f };

        bool CreateResources();
        void DestroyResources();
    };

} // namespace Engine::Graphics
