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
    wc.hCursor = static_cast<HCURSOR>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON2), IMAGE_CURSOR, 32, 32, 0));
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
    : width(width), height(height)
{
    // Calculate window size based on client region size and adjust for the title
    RECT wr;
    wr.left = 100;
    wr.right = width + wr.left;
    wr.top = 100;
    wr.bottom = height + wr.top;
    // AdjustWindowRect returns an int instead of an HResult :^ )
    if ((AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE)) == 0) {
        throw CHWND_LAST_EXCEPT();
    }
    AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);
    // Create window & get hWnd
    hWnd = CreateWindow(
        WindowClass::GetName(), name,
        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
        // this is lpCreateParams from tagCREATESTRUCT
        nullptr, nullptr, WindowClass::GetInstance(), this
    );
    // If error when making window
    if (hWnd == nullptr) {
        throw CHWND_LAST_EXCEPT();
    }
    // Newly created window starts off hidden
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    // Create Graphics object now that we have a handle to the window
    pGfx = std::make_unique<Graphics>(hWnd);
}

Window::~Window()
{
    DestroyWindow(hWnd);
}

void Window::SetTitle(const std::string title)
{
    if (SetWindowText(hWnd, title.c_str()) == 0) {
        throw CHWND_LAST_EXCEPT();
    }
}

std::optional<int> Window::ProcessMessage()
{
    MSG msg;
    // Doesn't block queue to wait for messages like GetMessage
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        // Check for quit message because PeekMessage does not signal that in the return
        if(msg.message == WM_QUIT ) {
            return msg.wParam;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Return an empty optional when not quitting the app
    return {};
}

Graphics& Window::Gfx()
{
    return *pGfx;
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
        // Forward message to regular handler
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
        // Handle Zombie key pressed when window is out of focus. "Send the key releseased message that was mistakingly sent to 
        // whatever took focus away."
    case WM_KILLFOCUS:
        kbd.ClearState();
        break;

        // Keybord related messages ///////////////////////////////////
    case WM_KEYDOWN:
        // Syskeydown needs to be handled here to track ALT (VK_MENU) key which is not part of KEYDOWN :^ ). 
    case WM_SYSKEYDOWN:
        if (!(lParam & 0x40000000) || kbd.AutorepeatIsEnabled()) {
            kbd.OnKeyPressed(static_cast<unsigned char>(wParam));
        }
        break;
    case WM_KEYUP:
        // Syskeyup needs to be handled here to track ALT key. 
    case WM_SYSKEYUP:
        kbd.OnKeyReleased(static_cast<unsigned char>(wParam));
        break;
    case WM_CHAR:
        kbd.OnChar(static_cast<unsigned char>(wParam));
        break;

        // Mouse Related Messages ///////////////////////////////////////
    case WM_MOUSEMOVE: 
    {
        const POINTS pt = MAKEPOINTS(lParam);
        // If the Mouse is in the client region. Procedure for mouse capture
        if (pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height) {
            mouse.OnMouseMove(pt.x, pt.y);
            if (!mouse.IsInWindow()) {
                SetCapture(hWnd);
                mouse.OnMouseEnter();
            }
        }
        // Mouse not in client region. Continue capture if buttons are being pressed
        else {
            if (wParam & (MK_LBUTTON | MK_RBUTTON)) {
                mouse.OnMouseMove(pt.x, pt.y);
            }
            // Not buttons pressed. Release capture
            else {
                ReleaseCapture();
                mouse.OnMouseLeave();
            }
        }
        break;
    }
    case WM_LBUTTONDOWN: 
    {
        const POINTS pt = MAKEPOINTS(lParam);
        mouse.OnLeftPressed(pt.x, pt.y);
        break;
    }
    case WM_RBUTTONDOWN: 
    {
        const POINTS pt = MAKEPOINTS(lParam);
        mouse.OnRightPressed(pt.x, pt.y);
        break;
    }
    case WM_LBUTTONUP: 
    {
        const POINTS pt = MAKEPOINTS(lParam);
        mouse.OnLeftReleased(pt.x, pt.y);
        break;
    }
    case WM_RBUTTONUP: 
    {
        const POINTS pt = MAKEPOINTS(lParam);
        mouse.OnRightReleased(pt.x, pt.y);
        break;
    }
    case WM_MOUSEHWHEEL:
    {
        const POINTS pt = MAKEPOINTS(lParam);
        const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        mouse.OnWheelDelta(pt.x, pt.y, delta);
        break;
    }
    }

    //Don't destroy window in the handler, only in the destructor
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