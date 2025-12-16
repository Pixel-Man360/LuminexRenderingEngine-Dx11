#include "Input.h"

using namespace Engine::Core;

HWND Input::s_hwnd = nullptr;
POINT Input::s_lastMouse{};
float Input::s_dx = 0;
float Input::s_dy = 0;

void Input::Initialize(HWND hwnd)
{
    s_hwnd = hwnd;
    GetCursorPos(&s_lastMouse);
    ScreenToClient(hwnd, &s_lastMouse);
}

void Input::Update()
{
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(s_hwnd, &p);

    s_dx = float(p.x - s_lastMouse.x);
    s_dy = float(p.y - s_lastMouse.y);

    s_lastMouse = p;
}

bool Input::IsKeyDown(int key)
{
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

bool Input::IsMouseButtonDown(int button)
{
    return (GetAsyncKeyState(button) & 0x8000) != 0;
}


float Input::GetMouseDeltaX() { return s_dx; }
float Input::GetMouseDeltaY() { return s_dy; }
