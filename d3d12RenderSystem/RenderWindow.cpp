#include "RenderWindow.h"

#include <functional>

using std::wstring;

namespace d3d12RenderSystem {

wstring const RenderWindow::windowClassName = L"RenderWindowClass";

LRESULT CALLBACK RenderWindow::RenderWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    auto *currentRenderWindow = reinterpret_cast<RenderWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    switch (uMsg) {
    case WM_CREATE:
        break;
    case WM_CLOSE:
        //MessageBox(nullptr, L"close", L"close?", 0);
        // todo: prompt for confirmation
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_SIZE:
    case WM_MOUSEWHEEL:
    case WM_MOUSEMOVE:
    case WM_KEYUP:
    case WM_KEYDOWN:
        if (currentRenderWindow->_inputHandler) {
            currentRenderWindow->_inputHandler(hWnd, uMsg, wParam, lParam);
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void RenderWindow::RegisterRenderWindowClass() {
    WNDCLASSEX renderWindowClass = {};
    renderWindowClass.cbSize = sizeof(WNDCLASSEX);
    renderWindowClass.lpfnWndProc = RenderWindowProc;

    renderWindowClass.hInstance = nullptr;
    renderWindowClass.lpszClassName = windowClassName.c_str();
    if (!RegisterClassEx(&renderWindowClass)) {
        DWORD errorCode = GetLastError(); // log this error
        abort();
    }
}

void RenderWindow::UnregisterRenderWindowClass() {
    if (!UnregisterClass(windowClassName.c_str(), nullptr)) {
        DWORD errorCode = GetLastError();
        abort(); //log this error
    }
}

void RenderWindow::CreateRenderWindow() {
    RECT rect{ 100, 100, _width + 100, _height + 100 };
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, 0); // calculate actual window rect according to client area rect
    _hwnd = CreateWindowEx(
        0,
        windowClassName.c_str(),
        _windowName.c_str(),
        WS_OVERLAPPEDWINDOW,
        rect.left,
        rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        nullptr,
        nullptr
        );

    // Use SetWIndowLongPtr to store a pointer to our class, 
    // so our (static) message handle function RenderWindowProc() can
    // retrieve this pointer by GetWindowsLongPtr and call input handle function
    // (which is a non-static member function)
    SetWindowLongPtr(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    if (!_hwnd) {
        DWORD errorCode = GetLastError(); // log this error
    }

    ShowWindow(_hwnd, SW_SHOW);
    UpdateWindow(_hwnd);
}

bool RenderWindow::Step() {
    //SwapBuffers(m_DeviceContextHandle);

    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            _hwnd = nullptr;
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);

    }
    return true;
}

RenderWindow::RenderWindow()
    : _hwnd(nullptr)
    , _windowName(L"")
    , _width(0)
    , _height(0) {
    RegisterRenderWindowClass();
}

void RenderWindow::RegisterInputHandler(std::function<void(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)> inputHandler) {
    _inputHandler = inputHandler;
}

void RenderWindow::Create(int width, int height, wstring name) {
    _width = width;
    _height = height;
    _windowName = name;

    CreateRenderWindow();
    //InitializeGl();
}

void RenderWindow::SetCaption(wstring text) {
    SetWindowText(_hwnd, text.c_str());
}

RenderWindow::~RenderWindow() {
    //DeinitializeGl();
    UnregisterRenderWindowClass();
}

}