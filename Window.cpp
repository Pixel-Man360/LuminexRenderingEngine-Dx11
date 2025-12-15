#include "Window.h"
#include "resource.h"
#include <cassert>
#include <string>

using namespace Engine::Core;

namespace
{
    const wchar_t* WIN_CLASS_NAME = L"SimpleEngineWindowClass";
}

Window::Window() = default;

Window::~Window()
{
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}

bool Window::Create(const wchar_t* windowTitle, int width, int height, HINSTANCE hInstance)
{
    m_width = width;
    m_height = height;
    m_hInstance = hInstance;

    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = Window::StaticWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(void*);
    wcex.hInstance = hInstance;
	wcex.hIcon = static_cast<HICON>(LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EngineIcon)));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = WIN_CLASS_NAME;
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassExW(&wcex))
    {
        return false;
    }

    RECT rc = { 0, 0, width, height };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    m_hWnd = CreateWindowW(
        WIN_CLASS_NAME,
        windowTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInstance, this
    );

    if (!m_hWnd)
    {
        return false;
    }

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    return true;
}

void Window::ProcessEvents()
{
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool Window::ShouldClose() const
{
    return m_shouldClose;
}

LRESULT CALLBACK Window::StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE)
    {
        CREATESTRUCT* pcs = reinterpret_cast<CREATESTRUCT*>(lParam);
        Window* pWindow = reinterpret_cast<Window*>(pcs->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
        return pWindow->WndProc(hWnd, message, wParam, lParam);
    }

    Window* pThis = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (pThis)
    {
        return pThis->WndProc(hWnd, message, wParam, lParam);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CLOSE:
    case WM_DESTROY:
    {
        m_shouldClose = true;
        PostQuitMessage(0);
        return 0;
    }

    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        m_width = width;
        m_height = height;
        if (m_resizeCallback)
        {
            m_resizeCallback(width, height);
        }
        return 0;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
