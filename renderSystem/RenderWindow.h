#pragma once

#include <Windows.h>

#include <string>
#include <functional>

using std::wstring;

class RenderWindow
{
public:
    RenderWindow();
    RenderWindow(RenderWindow const&) = delete;
    RenderWindow& operator= (RenderWindow const&) = delete;
    ~RenderWindow();

public:
	void RegisterKeyHandler(std::function<void(int)> keyHandler);
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
    static wstring const windowClassName;

    int m_Width;
    int m_Height;
    wstring m_WindowName;

    HWND m_RenderWindowHandle;
    HDC m_DeviceContextHandle;
    HGLRC m_GlRenderContextHandle;
	std::function<void(int)> _keyHandler;
};
