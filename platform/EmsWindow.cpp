#include "EmsWindow.hpp"
#include "Sdl.hpp"
#include "../graphics/Presenter.hpp"
#include "../input/SdlManager.hpp"
#include "../Exception.hpp"
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <SDL/SDL_video.h>

BEGIN_INANITY_PLATFORM

EmsWindow* EmsWindow::instance = 0;

EmsWindow::EmsWindow(const String& title, int width, int height)
: title(title), width(width), height(height)
{
	if(instance)
		THROW("Only one emscripten window is allowed");

	instance = this;

	// in emscripten all simplified
	// this is needed to initialize mouse input events
	sdl = Sdl::Get();
	SDL_SetVideoMode(0, 0, 0, SDL_OPENGL);
}

EmsWindow::~EmsWindow()
{
	instance = 0;
}

void EmsWindow::SetInputManager(ptr<Input::SdlManager> inputManager)
{
	this->inputManager = inputManager;
}

int EmsWindow::GetWidth() const
{
	return width;
}

int EmsWindow::GetHeight() const
{
	return height;
}

void EmsWindow::SetTitle(const String& title)
{
	this->title = title;
}

void EmsWindow::Close()
{
	emscripten_cancel_main_loop();
}

void EmsWindow::SetFullScreen(bool fullscreen)
{
	// not implemented yet
}

void EmsWindow::GetRect(int& left, int& top, int& width, int& height)
{
	left = 0;
	top = 0;
	width = this->width;
	height = this->height;
}

void EmsWindow::Run(ptr<Handler> activeHandler)
{
	this->activeHandler = activeHandler;
	emscripten_set_main_loop(&StaticMainLoop, 0, 0);
}

void EmsWindow::PlaceCursor(int x, int y)
{
	// not implemented yet
}

void EmsWindow::UpdateMouseLock()
{
	// not implemented yet
}

void EmsWindow::UpdateCursorVisible()
{
	// not implemented yet
}

void EmsWindow::StaticMainLoop()
{
	instance->MainLoop();
}

void EmsWindow::MainLoop()
{
	// update size
	int newWidth, newHeight;
	emscripten_get_canvas_element_size(nullptr, &newWidth, &newHeight);
	if(newWidth != width || newHeight != height)
	{
		width = newWidth;
		height = newHeight;
		if(presenter)
			presenter->Resize(width, height);
	}

	SDL_Event event;
	while(SDL_PollEvent(&event))
		if(inputManager)
			inputManager->ProcessEvent(event);

	if(inputManager)
		inputManager->Update();
	activeHandler->Fire();
}

ptr<Window::Icon> EmsWindow::CreateIcon(ptr<Graphics::RawTextureData> texture)
{
	return NEW(Icon());
}

void EmsWindow::SetIcon(ptr<Icon> icon)
{
}

ptr<Window::Cursor> EmsWindow::CreateCursor(ptr<Graphics::RawTextureData> texture, int hotX, int hotY)
{
	return NEW(Cursor());
}

void EmsWindow::SetCursor(ptr<Cursor> cursor)
{
}

END_INANITY_PLATFORM
