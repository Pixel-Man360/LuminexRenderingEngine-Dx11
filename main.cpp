#include <Windows.h>
#include <iostream>
#include "Window.h"
#include "DeviceResources.h"
#include "Renderer.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    Engine::Core::Window window;
    if (!window.Create(L"DX11 Graphics Engine", 1280, 720, hInstance))
    {
        MessageBox(nullptr, L"Failed to create window", L"Error", MB_OK);
        return -1;
    }

    Engine::Graphics::DeviceResources deviceResources;
    if (!deviceResources.Initialize(window.GetHwnd(), window.GetWidth(), window.GetHeight()))
    {
        MessageBox(nullptr, L"Failed to initialize Direct3D", L"Error", MB_OK);
        return -1;
    }

    window.SetResizeCallback([&deviceResources](int w, int h)
        {
            deviceResources.Resize(w, h);
        });

    Engine::Graphics::Renderer renderer;
    if (!renderer.Initialize(&deviceResources))
    {
        MessageBox(nullptr, L"Failed to initialize renderer", L"Error", MB_OK);
        return -1;
    }

    renderer.SetClearColor(0.247f, 0.557f, 0.651f, 1.0f);

    while (!window.ShouldClose())
    {
        window.ProcessEvents();
        renderer.Render();
        // Sleep(1); // optional, to reduce CPU usage
    }

    return 0;
}
