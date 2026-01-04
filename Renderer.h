#pragma once

#include <DirectXMath.h>
#include <vector>
#include "DeviceResources.h"
#include "Camera.h"
#include "RenderObject.h"
#include "Light.h"
#include "CBLight.h"
#include "ConstantBuffer.h"



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
		Shader* m_shadowShader = nullptr;
        Shader* m_shadowDebugShader = nullptr;
        Mesh* m_mesh = nullptr;
		Mesh* m_planeMesh = nullptr;
        ConstantBuffer* m_cbPerObject = nullptr;
        ConstantBuffer* m_cbLight = nullptr;
        vector<Light> m_lights;



        // Pipeline states
		ID3D11RasterizerState* m_rasterizerState = nullptr;
		ID3D11RasterizerState* m_shadowRasterizerState = nullptr;
		ID3D11DepthStencilState* m_depthStencilState = nullptr;

        // Texture
        ID3D11ShaderResourceView* m_brickTexture = nullptr;
        ID3D11ShaderResourceView* m_groundTexture = nullptr;
        ID3D11SamplerState* m_samplerState = nullptr;
		ID3D11Texture2D* m_shadowMapTexture = nullptr;
		ID3D11DepthStencilView* m_shadowMapDSV = nullptr;
		ID3D11ShaderResourceView* m_shadowMapSRV = nullptr;
		ID3D11SamplerState* m_shadowMapSampler = nullptr;

        // Shadow matrices
        XMMATRIX m_lightView;
        XMMATRIX m_lightProj;

        // Shadow debug
        bool m_showShadowDebug = false;

        // Debug quad
        ID3D11Buffer* m_fullscreenVB = nullptr;

       
     
        Camera m_camera;

		vector<RenderObject*> m_renderObjects;


        XMFLOAT4 m_clearColor{ 0.1f, 0.5f, 0.6f, 1.0f };

        bool CreateResources();
        void ShadowPass();
        void MainRenderPass();
        void RenderShadowDebug();
        void ToggleShadowDebug() { m_showShadowDebug = !m_showShadowDebug; }
        void DestroyResources();
    };

} // namespace Engine::Graphics
