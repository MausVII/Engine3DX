#include "Window.h"
#include <sstream>
#include "resource.h"

Window::WindowClass Window::WindowClass::wndClass;

Window::WindowClass::WindowClass() noexcept
    :
    hInst(GetModuleHandle(nullptr))
{
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = HandleMsgSetup;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetInstance();
    wc.hIcon = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 128, 128, 0));
    wc.hCursor = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = GetName();
    wc.hIconSm = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 128, 128, 0));
    RegisterClassEx(&wc);
}

const char* Window::WindowClass::GetName() noexcept
{
    return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept
{
    return wndClass.hInst;
}

Window::WindowClass::~WindowClass()
{
    UnregisterClass(wndClassName, GetInstance());
}

Window::Window(int width, int height, const char* name) 
{
    // Calculate window size based on client region size
    RECT wr;
    wr.left = 100;
    wr.right = width + wr.left;
    wr.top = 100;
    wr.bottom = height + wr.top;
    if (FAILED(AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE))) {
        throw CHWND_LAST_EXCEPT();
    }
    AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);
    // Create window & get hWnd
    hWnd = CreateWindow(
        WindowClass::GetName(), name,
        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr, WindowClass::GetInstance(), this
    );
    // If error when making window
    if (hWnd == nullptr) {
        throw CHWND_LAST_EXCEPT();
    }
    // Show window
    ShowWindow(hWnd, SW_SHOWDEFAULT);
}

Window::~Window()
{
    DestroyWindow(hWnd);
}

LRESULT WINAPI Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    // Use Create Parameter from CreateWindow to store Window Class pointer
    if (msg == WM_NCCREATE) {
        // Extract pointer to Window Class from Creation Data
        const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
        // Set WinAPI-managed user data to store pointer to Win Class
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
        // Set message proc to normal handler once setup is finished
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
        // Forward message to window class handler
        return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
    }
    // If we get a message before WM_NCCREATE message, handle with default
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    // Retrieve pointer to window class
    Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    // Forward message to window class handler
    return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (msg) {
    case WM_CLOSE:
        PostQuitMessage(0);
        // Call OUR destructor, don't destroy window twice
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Exception related ///////////////////////////////////////////////////////////////

Window::Exception::Exception(int line, const char* file, HRESULT hr) noexcept
    :
    WinExcept( line, file),
    hr(hr)
{
}

const char* Window::Exception::what() const noexcept
{
    std::ostringstream oss;
    oss << GetType() << std::endl
        << "[Error Code] " << GetErrorCode() << std::endl
        << "[Description] " << GetErrorString() << std::endl
        << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* Window::Exception::GetType() const noexcept
{
    return "Engine 3DX Window Exception";
}

std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept
{
    char* pMsgBuffer = nullptr;
    // Windows will allocate memory for error string and make a pointer for it
    DWORD nMsgLen = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&pMsgBuffer), 0, nullptr
    );

    // Lenghtless string indicates a failure
    if (nMsgLen == 0) {
        return "Unindentifeid error code";
    }

    // Copy error string from windows-allocated buffer to std::string
    std::string errorString = pMsgBuffer;
    // Free buffer
    LocalFree(pMsgBuffer);
    return errorString;
}

HRESULT Window::Exception::GetErrorCode() const noexcept
{
    return hr;
}

std::string Window::Exception::GetErrorString() const noexcept
{
    return TranslateErrorCode(hr);
}

//////////////////////////////////////////////////////////////////