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
#include <fstream>
#include "sdl_api.h"
#include "sdl_image_api.h"
#include "smart_pointers.h"
#include "helpers.h"
#include "SDL.h"
#include "SDL_image.h"
#include <stdexcept>
#include "scene.h"
#include "ui_object.h"

using namespace std;
using namespace foo;

struct RendererObject {
	TexturePtr texture;
	SDL_Rect position;
	int repeat_x;
	int repeat_y;
};

RendererObject
LoadRendererObject(
	SDL_Renderer *renderer,
	const UiObject &from);

void
ProcessScene(
	SDL_Renderer *renderer,
	SDL_Window *window,
	const Scene& scene,
	vector<RendererObject> &renderer_objects);

void
RenderScene(
	SDL_Renderer *renderer,
	const vector<RendererObject> &renderer_objects);

int
main(int argc, char** argv) {
	Scene main_scene = LoadSceneFromFile("assets/scene.json");

	SdlApi sdl_api(SDL_INIT_VIDEO);
	SdlImageApi sdl_image_api(IMG_INIT_PNG);

	WindowPtr main_window(SDL_CreateWindow(
		main_scene.title.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		main_scene.width,
		main_scene.height,
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

	vector<RendererObject> renderer_objects;
	ProcessScene(renderer.get(), nullptr, main_scene, renderer_objects);

	bool is_running = true;
	while (is_running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				is_running = false;
				break;
			} else if (event.type == SDL_KEYDOWN) {
				if (event.key.repeat) continue;
				if (event.key.keysym.sym == SDLK_F5) {
					main_scene = LoadSceneFromFile("assets/scene.json");
					renderer_objects.clear();
					ProcessScene(
						renderer.get(),
						main_window.get(),
						main_scene, renderer_objects);
				}
			}
		}

		SDL_RenderClear(renderer.get());

		RenderScene(renderer.get(), renderer_objects);

		SDL_RenderPresent(renderer.get());
	}

	return 0;
}

void
ProcessScene(
		SDL_Renderer *renderer,
		SDL_Window *window,
		const Scene& scene,
		vector<RendererObject> &renderer_objects) {
	if (window) {
		SDL_SetWindowTitle(window, scene.title.c_str());
		SDL_SetWindowSize(window, scene.width, scene.height);
		SDL_SetWindowPosition(
			window,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED);
	}

	for (const auto &obj: scene.objects) {
		renderer_objects.emplace_back(
			LoadRendererObject(renderer, obj));
	}
}

void
RenderScene(
		SDL_Renderer *renderer,
		const vector<RendererObject> &renderer_objects) {

	for (const auto& obj: renderer_objects) {

		for (int y = 0; y < obj.repeat_y; ++y) {
			SDL_Rect rt2 = obj.position;
			rt2.y = obj.position.y + obj.position.h * y;

			for (int x = 0; x < obj.repeat_x; ++x) {
				rt2.x = obj.position.x + obj.position.w * x;
				SDL_RenderCopy(
					renderer,
					obj.texture.get(),
					nullptr,
					&rt2);
			}
		}
	}
}

RendererObject
LoadRendererObject(
		SDL_Renderer *renderer,
		const UiObject &from) {
	SurfacePtr cpu_mem(IMG_Load(from.path.c_str()));
	if (!cpu_mem) {
		throw runtime_error(IMG_GetError());
	}

	RendererObject to;
	to.position.x = from.x;
	to.position.y = from.y;
	to.repeat_x = from.repeat_x;
	to.repeat_y = from.repeat_y;
	to.position.w = cpu_mem->w;
	to.position.h = cpu_mem->h;
	to.texture = TexturePtr(SDL_CreateTextureFromSurface(
		renderer, cpu_mem.get()));
	if (!to.texture) {
		throw runtime_error(SDL_GetError());
	}

	return move(to);
}