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
        DirectX::XMFLOAT4 m_clearColor{ 0.1f, 0.3f, 0.6f, 1.0f };

        Shader* m_shader = nullptr;
        Mesh* m_mesh = nullptr;
        ConstantBuffer* m_cb = nullptr;

        float m_rotationAngle = 0.0f;

        //Camera Data
		XMFLOAT3 m_cameraPosition = XMFLOAT3(0.0f, 0.0f, -2.0f);
		XMFLOAT3 m_cameraTarget = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 m_cameraUp = XMFLOAT3(0.0f, 1.0f, 0.0f);

        bool CreateResources();
        void DestroyResources();
    };

} // namespace Engine::Graphics
