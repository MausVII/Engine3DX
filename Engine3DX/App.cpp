#include "App.h"

App::App()
	:
	wnd(800, 600, "Engine3DX")
{}

int App::Go()
{
	while (true) {
		// Process all messages pending, but don't block
		if (const auto ecode = Window::ProcessMessage()) {
			// If optional has value, that is a quit message
			return *ecode;
		}
		DoFrame();
	}
}

void App::DoFrame()
{
	wnd.Gfx().ClearBuffer(0.0f, 100.0f, 100.0f);
	wnd.Gfx().EndFrame();
}