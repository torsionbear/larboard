#pragma once

#include <Windows.h>
#include <Windowsx.h>

#include <string>
#include <functional>

namespace d3d12RenderSystem {

class RenderWindow {
public:
    RenderWindow();
    RenderWindow(RenderWindow const&) = delete;
    RenderWindow& operator= (RenderWindow const&) = delete;
    ~RenderWindow();

public:
    auto RegisterInputHandler(std::function<void(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)> inputHandler) -> void;
    auto Create(int width, int height, std::wstring name) -> void;
    auto SetCaption(std::wstring name) -> void;
    auto Step() -> bool;
    auto GetWidth() -> int {
        return _width;
    }
    auto GetHeight() -> int {
        return _height;
    }
    auto GetHwnd() -> HWND {
        return _hwnd;
    }

private:
    static LRESULT CALLBACK RenderWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void RegisterRenderWindowClass();
    void UnregisterRenderWindowClass();

    void CreateRenderWindow();
    //void InitializeGl();
    //void DeinitializeGl();

private:
    static std::wstring const windowClassName;

    int _width;
    int _height;
    std::wstring _windowName;

    HWND _hwnd;
    //HDC m_DeviceContextHandle;
    std::function<void(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)> _inputHandler;
};

}