#pragma once
#include <Windows.h>

namespace Engine::Core
{
    class Input
    {
    public:
        static void Initialize(HWND hwnd);
        static void Update();

        static bool IsKeyDown(int key);
        static bool IsMouseButtonDown(int button);

        static float GetMouseDeltaX();
        static float GetMouseDeltaY();

        static float GetMouseWheelDelta();
        static void OnMouseWheelMoved(float delta);
		static void ResetMouseWheelDelta() { s_wheelDelta = 0.0f; }

    private:
        static HWND s_hwnd;
        static POINT s_lastMouse;
        static float s_dx;
        static float s_dy;
        static float s_wheelDelta;
    };
}
