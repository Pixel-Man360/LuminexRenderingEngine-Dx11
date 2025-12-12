#include "Shader.h"
#include <stdexcept>

using namespace Engine::Graphics;

static bool CompileShaderFromFile(const wchar_t* filePath, const char* entryPoint, const char* target, std::vector<BYTE>& outBlob)
{
    outBlob.clear();

    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(filePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, target, compileFlags, 0, &shaderBlob, &errorBlob);

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
            errorBlob->Release();
        }
        if (shaderBlob) shaderBlob->Release();
        return false;
    }

    // copy blob bytes into vector for storage
    outBlob.resize(shaderBlob->GetBufferSize());
    memcpy(outBlob.data(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());

    shaderBlob->Release();
    if (errorBlob) errorBlob->Release();
    return true;
}

bool Shader::LoadFromFiles(ID3D11Device* device, const wchar_t* vsPath, const wchar_t* psPath, const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT layoutNumElements)
{
    if (!device) return false;

    // Compile vertex shader
    if (!CompileShaderFromFile(vsPath, "main", "vs_5_0", m_vsBlob))
    {
        return false;
    }

    // Create vertex shader object from compiled blob
    HRESULT hr = device->CreateVertexShader(m_vsBlob.data(), m_vsBlob.size(), nullptr, m_vs.GetAddressOf());
    if (FAILED(hr)) return false;

    // Create input layout using the vertex shader blob (the blob contains signature)
    hr = device->CreateInputLayout(layoutDesc, layoutNumElements, m_vsBlob.data(), m_vsBlob.size(), m_inputLayout.GetAddressOf());
    if (FAILED(hr)) return false;

    // Compile pixel shader
    std::vector<BYTE> psBlob;
    if (!CompileShaderFromFile(psPath, "main", "ps_5_0", psBlob))
    {
        return false;
    }

    // Create pixel shader object
    hr = device->CreatePixelShader(psBlob.data(), psBlob.size(), nullptr, m_ps.GetAddressOf());
    if (FAILED(hr)) return false;

    return true;
}

void Shader::Bind(ID3D11DeviceContext* context)
{
    if (!context) return;

    context->IASetInputLayout(m_inputLayout.Get());
    context->VSSetShader(m_vs.Get(), nullptr, 0);
    context->PSSetShader(m_ps.Get(), nullptr, 0);
}

void Shader::Release()
{
    m_inputLayout.Reset();
    m_vs.Reset();
    m_ps.Reset();
    m_vsBlob.clear();
}
