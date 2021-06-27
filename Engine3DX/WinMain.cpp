#include "Window.h"
#include "WinExcept.h"

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
)

{
	try {
		Window wnd(1280, 720, "The Window");

		MSG msg;
		BOOL gResult;
		while ((gResult = GetMessage(&msg, nullptr, 0, 0)) > 0) {
			// Translate message posts auxilliary WM_CHAR messages from the key msgs
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (gResult == -1) {
			return -1;
		}

		return msg.wParam;
	}
	catch (const WinExcept& e){
		MessageBox(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	}
	catch (const std::exception& e) {
		MessageBox(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	catch (...) {
		MessageBox(nullptr, "No details available", "Unknown exception", MB_OK | MB_ICONEXCLAMATION);
	}
	return -1;

}