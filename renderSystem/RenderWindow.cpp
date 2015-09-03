#include "RenderWindow.h"

#include <functional>

wstring const RenderWindow::windowClassName = L"RenderWindowClass";

LRESULT CALLBACK RenderWindow::RenderWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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
	case WM_SIZE:
	case WM_MOUSEWHEEL:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
		if (currentRenderWindow->_inputHandler) {
			currentRenderWindow->_inputHandler(hWnd, uMsg, wParam, lParam);
		}
	}
	
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void RenderWindow::RegisterRenderWindowClass()
{
    WNDCLASSEX renderWindowClass = {};
    renderWindowClass.cbSize = sizeof(WNDCLASSEX);
	renderWindowClass.lpfnWndProc = RenderWindowProc;

    renderWindowClass.hInstance = nullptr;
    renderWindowClass.lpszClassName = windowClassName.c_str();
    if (!RegisterClassEx(&renderWindowClass))
    {
        DWORD errorCode = GetLastError(); // log this error
        abort();
    }
}

void RenderWindow::UnregisterRenderWindowClass()
{
    if (!UnregisterClass(windowClassName.c_str(), nullptr))
    {
        DWORD errorCode = GetLastError();
        abort(); //log this error
    }
}

void RenderWindow::CreateRenderWindow()
{
    m_RenderWindowHandle = CreateWindowEx(
        0,
        windowClassName.c_str(),
        m_WindowName.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        m_Width,
        m_Height,
        nullptr,
        nullptr,
        nullptr,
        nullptr
        );

	// Use SetWIndowLongPtr to store a pointer to our class, 
	// so our (static) message handle function RenderWindowProc() can
	// retrieve this pointer by GetWindowsLongPtr and call input handle function
	// (which is a non-static member function)
	SetWindowLongPtr(m_RenderWindowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    if (!m_RenderWindowHandle)
    {
        DWORD errorCode = GetLastError(); // log this error
    }

    ShowWindow(m_RenderWindowHandle, SW_SHOW);
    UpdateWindow(m_RenderWindowHandle);
}

void RenderWindow::InitializeGl()
{
    m_DeviceContextHandle = GetDC(m_RenderWindowHandle);
    // setup pixel format before creating gl render context
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);		// size
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_GENERIC_ACCELERATED;
    pfd.nVersion = 1;				// version: always set to 1
    pfd.iPixelType = PFD_TYPE_RGBA;	// color type
    pfd.cColorBits = 32;			// color depth
    pfd.cDepthBits = 24;			// depth buffer
    pfd.iLayerType = PFD_MAIN_PLANE;// main layer
    int pixelFormat = ChoosePixelFormat(m_DeviceContextHandle, &pfd);
    SetPixelFormat(m_DeviceContextHandle, pixelFormat, &pfd);
    // create gl render context
    m_GlRenderContextHandle = wglCreateContext(m_DeviceContextHandle);
    if (m_GlRenderContextHandle == nullptr)
    {
        DWORD errorCode = GetLastError(); // log this error
    }
    // select gl render context
    wglMakeCurrent(m_DeviceContextHandle, m_GlRenderContextHandle);
}

void RenderWindow::DeinitializeGl()
{
    // deselect gl render context
    wglMakeCurrent(m_DeviceContextHandle, NULL);
    // delete gl render context
    wglDeleteContext(m_GlRenderContextHandle);
}

bool RenderWindow::Step()
{
    SwapBuffers(m_DeviceContextHandle);

    MSG msg;
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            m_RenderWindowHandle = nullptr;
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);

    }
    return true;
}

RenderWindow::RenderWindow()
    : m_RenderWindowHandle(nullptr)
    , m_WindowName(L"")
    , m_Width(0)
    , m_Height(0)
{
    RegisterRenderWindowClass();
}

void RenderWindow::RegisterInputHandler(std::function<void(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)> inputHandler) {
	_inputHandler = inputHandler;
}

void RenderWindow::Create(int width, int height, wstring name)
{
    m_Width = width;
    m_Height = height;
    m_WindowName = name;

    CreateRenderWindow();
    InitializeGl();
}

RenderWindow::~RenderWindow()
{
    DeinitializeGl();
    UnregisterRenderWindowClass();
}
