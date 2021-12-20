#pragma once
#include "Window.h"

class App {
public:
	App();
	// Message Loop
	int Go();
private:
	void DoFrame();
private:
	Window wnd;
};