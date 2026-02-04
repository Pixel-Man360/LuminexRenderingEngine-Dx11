#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace Engine::Graphics
{

    class DeviceResources; // forward-declared

    class Shader
    {
    public:
        Shader() = default;
        ~Shader() = default;

        // Load and compile a vertex + pixel shader from files.
        bool LoadFromFiles(ID3D11Device* device, const wchar_t* vsPath, const wchar_t* psPath,
            const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT layoutNumElements);

        // Bind shader + input layout to the pipeline
        void Bind(ID3D11DeviceContext* context);

        void Release();

    private:
        ComPtr<ID3D11VertexShader> m_vs;
        ComPtr<ID3D11PixelShader> m_ps;
        ComPtr<ID3D11InputLayout> m_inputLayout;
		std::vector<BYTE> m_vsBlob; // keep compiled VS blob for input layout creation
    };

} // namespace Engine::Graphics
