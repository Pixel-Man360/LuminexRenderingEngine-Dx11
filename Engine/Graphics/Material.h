#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;


namespace Engine::Graphics
{
	struct PBRMaterialData
	{
		DirectX::XMFLOAT4 Albedo = { 1.0f, 1.0f, 1.0f, 1.0f }; // RGBA color
		float Metallic = 0.0f; // 0.0 = non-metal, 1.0 = metal
		float Roughness = 0.5f; // 0.0 = smooth, 1.0 = rough
		float AO = 1.0f; 
		float padding; 

		DirectX::XMFLOAT2 Tiling = { 1.0f, 1.0f };
		DirectX::XMFLOAT2 Offset = { 0.0f, 0.0f };
	};
	class Material
	{
	public: 
		Material() = default;
		~Material() = default;

		void SetAlbedo(const DirectX::XMFLOAT4& albedo) { m_data.Albedo = albedo; }
		void SetMetallic(float metallic) { m_data.Metallic = metallic; }
		void SetRoughness(float roughness) { m_data.Roughness = roughness; }
		void SetAO(float ao) { m_data.AO = ao; }

		// Tiling / Offset
		void SetTiling(const DirectX::XMFLOAT2& tiling) { m_data.Tiling = tiling; }
		void SetOffset(const DirectX::XMFLOAT2& offset) { m_data.Offset = offset; }
		const DirectX::XMFLOAT2& GetTiling() const { return m_data.Tiling; }
		const DirectX::XMFLOAT2& GetOffset() const { return m_data.Offset; }


		// Texture maps (nullptr = use scalar value)
		void SetAlbedoMap(ID3D11ShaderResourceView* srv) { m_albedoMap = srv; }
		void SetNormalMap(ID3D11ShaderResourceView* srv) { m_normalMap = srv; }
		void SetMetallicMap(ID3D11ShaderResourceView* srv) { m_metallicMap = srv; }
		void SetRoughnessMap(ID3D11ShaderResourceView* srv) { m_roughnessMap = srv; }
		void SetAOMap(ID3D11ShaderResourceView* srv) { m_aoMap = srv; }

		const PBRMaterialData& GetData() const { return m_data; }

		ID3D11ShaderResourceView* GetAlbedoMap() const { return m_albedoMap; }
		ID3D11ShaderResourceView* GetNormalMap() const { return m_normalMap; }
		ID3D11ShaderResourceView* GetMetallicMap() const { return m_metallicMap; }
		ID3D11ShaderResourceView* GetRoughnessMap() const { return m_roughnessMap; }
		ID3D11ShaderResourceView* GetAOMap() const { return m_aoMap; }

	private:
		PBRMaterialData m_data;

		ID3D11ShaderResourceView* m_albedoMap = nullptr;
		ID3D11ShaderResourceView* m_normalMap = nullptr;
		ID3D11ShaderResourceView* m_metallicMap = nullptr;
		ID3D11ShaderResourceView* m_roughnessMap = nullptr;
		ID3D11ShaderResourceView* m_aoMap = nullptr;

	};
}

