#pragma once

#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include "DeviceResources.h"
#include "Mesh.h"
#include "../Core/Camera.h"
#include "../Scene/Scene.h"

#include "Light.h"
#include "CBLight.h"
#include "CBMaterial.h"
#include "Material.h"
#include "ConstantBuffer.h"


static const uint32_t NUM_CASCADES = 4;
static const uint32_t SHADOW_MAP_SIZE = 2048;

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

        void SetActiveScene(Engine::Scene::Scene* scene);
        void SetSelectedObject(Engine::Scene::SceneObject* obj);
        
        // Get available meshes for creating objects
        Mesh* GetCubeMesh() const { return m_cube; }
        Mesh* GetSphereMesh() const { return m_sphere; }
        Mesh* GetCylinderMesh() const { return m_cylinder; }
        Mesh* GetCapsuleMesh() const { return m_capsule; }
        Mesh* GetPlaneMesh() const { return m_planeMesh; }
        

        ID3D11ShaderResourceView* GetBrickTexture() const { return m_brickTexture; }
        ID3D11ShaderResourceView* GetGroundTexture() const { return m_groundTexture; }
        
        struct TextureInfo 
        { 
            std::string name; 
            ID3D11ShaderResourceView* srv; 
        };

        std::vector<TextureInfo> GetAvailableTextures() const { return m_textures; }

        enum class TextureColorSpace
        {
            SRGB,
            Linear
        };

        ID3D11ShaderResourceView* ImportTextureFromFile(const std::wstring& filepath, TextureColorSpace colorSpace);


        std::shared_ptr<Material> CreateDefaultMaterial();
        
    
        DirectX::XMMATRIX GetViewMatrix() const { return m_camera.GetViewMatrix(); }
        DirectX::XMMATRIX GetProjectionMatrix() const;
        DirectX::XMFLOAT3 GetCameraPosition() const { return m_camera.GetPosition(); }
        void FocusCameraOn(Engine::Scene::SceneObject* object);
        int GetScreenWidth() const { return static_cast<int>(m_deviceResources->GetWidth()); }
        int GetScreenHeight() const { return static_cast<int>(m_deviceResources->GetHeight()); }
        

        ID3D11ShaderResourceView* GetViewportSRV() const { return m_viewportSRV; }
        void ResizeViewport(int width, int height);
        int GetViewportWidth() const { return m_viewportWidth; }
        int GetViewportHeight() const { return m_viewportHeight; }
        
        // Bind viewport render target for external rendering (gizmo)
        void BindViewportRenderTarget();

        // Import mesh from file (returns nullptr on failure)
        Mesh* ImportMesh(const std::string& filepath);
        ID3D11Device* GetDevice() const;

        void ImportMeshAsync(const std::string& filepath);
        bool IsImportInProgress() const { return m_importInProgress.load(); }
        float GetImportProgress() const { return m_importProgress.load(); }
        bool HasImportResult() const { return m_hasImportResult.load(); }

        Mesh* FinalizePendingImport();

    private:
        void GatherLightsFromScene();
        bool CreateViewportRenderTarget(int width, int height);


        Engine::Scene::Scene* m_activeScene = nullptr;
        Engine::Scene::SceneObject* m_selectedObject = nullptr;

        DeviceResources* m_deviceResources = nullptr;
        Shader* m_shader = nullptr;
		Shader* m_shadowShader = nullptr;
        Shader* m_shadowDebugShader = nullptr;
        Mesh* m_cube = nullptr;
		Mesh* m_sphere = nullptr;
		Mesh* m_cylinder = nullptr;
		Mesh* m_capsule = nullptr;
		Mesh* m_planeMesh = nullptr;
        std::vector<std::unique_ptr<Mesh>> m_importedMeshes;
        ConstantBuffer* m_cbPerObject = nullptr;
        ConstantBuffer* m_cbLight = nullptr;
		ConstantBuffer* m_cbShadow = nullptr;
        ConstantBuffer* m_cbMaterial = nullptr;
        std::vector<Light> m_lights;



        // Pipeline states
		ID3D11RasterizerState* m_rasterizerState = nullptr;
		ID3D11RasterizerState* m_shadowRasterizerState = nullptr;
		ID3D11DepthStencilState* m_depthStencilState = nullptr;

        // Texture
        ID3D11ShaderResourceView* m_brickTexture = nullptr;
        ID3D11ShaderResourceView* m_groundTexture = nullptr;
        ID3D11SamplerState* m_samplerState = nullptr;

        // Dynamic list of available textures (includes defaults and imported textures)
        std::vector<TextureInfo> m_textures;

        ID3D11Texture2D* m_shadowMapArray = nullptr;
        ID3D11DepthStencilView* m_shadowMapDSVArray = nullptr;
        ID3D11ShaderResourceView* m_shadowMapSRVArray = nullptr;

		ID3D11SamplerState* m_shadowMapSampler = nullptr;

        // Shadow matrices
        DirectX::XMMATRIX m_lightViewProj[NUM_CASCADES];
        ID3D11DepthStencilView* m_shadowCascadeDSVs[NUM_CASCADES];

        float    m_cascadeSplits[NUM_CASCADES];
        float m_cascadeLambda = 0.6f;
        float m_nearZ = 0.1f;
        float m_farZ = 1000.0f;

        DirectX::XMMATRIX m_lightView;
        DirectX::XMMATRIX m_lightProj;


        // Shadow debug
        bool m_showShadowDebug = false;

        // Debug quad
        ID3D11Buffer* m_fullscreenVB = nullptr;

        // Viewport render target (for rendering scene to texture)
        ID3D11Texture2D* m_viewportTexture = nullptr;
        ID3D11RenderTargetView* m_viewportRTV = nullptr;
        ID3D11ShaderResourceView* m_viewportSRV = nullptr;
        ID3D11Texture2D* m_viewportDepthTexture = nullptr;
        ID3D11DepthStencilView* m_viewportDSV = nullptr;
        int m_viewportWidth = 1280;
        int m_viewportHeight = 720;
       
        Engine::Core::Camera m_camera;

        DirectX::XMFLOAT4 m_clearColor{ 0.1f, 0.5f, 0.6f, 1.0f };

        std::atomic<bool> m_importInProgress{ false };
        std::atomic<float> m_importProgress{ 0.0f };
        std::thread m_importThread;
        std::mutex m_importMutex;
        std::vector<Vertex> m_importVertices;
        std::vector<uint32_t> m_importIndices;
        std::atomic<bool> m_hasImportResult{ false };
        bool m_importSuccess = false;
        std::string m_importPath;

		ComPtr<ID3D11ShaderResourceView> m_irradianceMap;
		ComPtr<ID3D11ShaderResourceView> m_prefilterMap;
		ComPtr<ID3D11ShaderResourceView> m_brdfLUT;

        ComPtr<ID3D11SamplerState> m_iblSampler;

        bool CreateResources();
        void ShadowPass();
        void MainRenderPass();
        void RenderShadowDebug();
		void ComputeCascadeSplits();
        void ToggleShadowDebug() { m_showShadowDebug = !m_showShadowDebug; }
    };

} 
