#pragma once

#include <string>
#include <vector>
#include <d3d11.h>
#include <atomic>
#include "Mesh.h"

namespace Engine::Graphics
{
	class ModelLoader
	{
	public:
		static bool LoadFromFile(const std::string& filepath, ID3D11Device*device,
								 std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices,
								 std::atomic<float>* progress = nullptr);

		static bool CreateMeshFromFile(const std::string& filepath, ID3D11Device* device,
									   ComPtr<ID3D11Buffer>& vertexBuffer, ComPtr<ID3D11Buffer>& indexBuffer,
									   UINT& indexCount);
	};
}

