/*
Copyright (c) 2015 Dilyan Rusev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <iostream>
#include "sdl_api.h"
#include "sdl_image_api.h"
#include "smart_pointers.h"
#include "SDL.h"
#include "SDL_image.h"
#include <stdexcept>

using namespace std;
using namespace foo;

int main(int argc, char** argv) {
	SdlApi sdl_api(SDL_INIT_VIDEO);
	SdlImageApi sdl_image_api(IMG_INIT_PNG);

	WindowPtr main_window(SDL_CreateWindow(
		"Foo Asteroids",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640,
		480,
		0));
	if (!main_window) {
		throw runtime_error(SDL_GetError());
	}

	RendererPtr renderer(SDL_CreateRenderer(
		main_window.get(),
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
	if (!renderer) {
		throw runtime_error(SDL_GetError());
	}

	bool is_running = true;
	while (is_running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				is_running = false;
				break;
			}
		}

		SDL_RenderClear(renderer.get());
		SDL_RenderPresent(renderer.get());
	}

	return 0;
}