#pragma once

struct ID3D11Device;
struct ID3D11DeviceContext;
struct HWND__;


namespace Engine::Editor
{
	class Editor
	{
	public:
		void Initialize(HWND__* hwnd, ID3D11Device* device, ID3D11DeviceContext* context);

		void BeginFrame();
		void Draw();
		void EndFrame();

		void Shutdown();
	};
}


