
#include <Windows.h>

// Def Win Proc
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CLOSE:
		// Quit Code = 31
		PostQuitMessage(31);
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
)
{

	const auto pClassName = L"Engine3DX";
	// Configure Win Class
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = pClassName;
	wc.hIconSm = nullptr;

	// Register Win Class
	RegisterClassEx(&wc);

	// Create Win Instance
	HWND hWnd = CreateWindowEx(
		0, pClassName, L"Window",
		WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		300, 300, 1280, 720,
		nullptr, nullptr, hInstance, nullptr
	);
	ShowWindow(hWnd, SW_SHOW);

	// Message Handling
	MSG msg;
	BOOL gResult;
	while ((gResult = GetMessage(&msg, nullptr, 0, 0)) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Case Error
	if (gResult == -1) {
		return -1;
	}
	// Else exit code
	else {
		return msg.lParam;
	}
}
