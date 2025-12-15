#pragma once

#include <DirectXMath.h>
#include "DeviceResources.h"

using namespace DirectX;

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

        XMFLOAT4 m_clearColor{ 0.1f, 0.5f, 0.6f, 1.0f };
        float m_rotationAngle = 0.0f;

        bool CreateResources();
        void DestroyResources();
    };

} // namespace Engine::Graphics
