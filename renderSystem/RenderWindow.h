#pragma once

#include <Windows.h>

#include <string>

using std::wstring;

class RenderWindow
{
public:
    RenderWindow();
    RenderWindow(const RenderWindow&) = delete;
    RenderWindow& operator= (const RenderWindow&) = delete;
    ~RenderWindow();

    void Create(int width, int height, wstring name);
    bool Step();

private:
    static LRESULT CALLBACK RenderWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void RegisterRenderWindowClass();
    void UnregisterRenderWindowClass();

    void CreateRenderWindow();
    void InitializeGl();
    void DeinitializeGl();

private:
    static const wstring windowClassName;

    int m_Width;
    int m_Height;
    wstring m_WindowName;

    HWND m_RenderWindowHandle;
    HDC m_DeviceContextHandle;
    HGLRC m_GlRenderContextHandle;

};
