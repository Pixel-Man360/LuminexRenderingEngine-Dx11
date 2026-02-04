#pragma once

#include <Windows.h>
#include <functional>

namespace Engine::Core
{

    class DeviceResources; // forward

    class Window
    {
    public:
        Window();
        ~Window();

        bool Create(const wchar_t* windowTitle, int width, int height, HINSTANCE hInstance);
        void ProcessEvents();
        bool ShouldClose() const;

        HWND GetHwnd() const { return m_hWnd; }
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }

        void SetResizeCallback(const std::function<void(int, int)>& cb) { m_resizeCallback = cb; }

    private:
        static LRESULT CALLBACK StaticWndProc(HWND, UINT, WPARAM, LPARAM);
        LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

    private:
        HWND m_hWnd = nullptr;
        HINSTANCE m_hInstance = nullptr;
        bool m_shouldClose = false;
        int m_width = 1280;
        int m_height = 720;

        std::function<void(int, int)> m_resizeCallback;
    };

} // namespace Engine::Core
