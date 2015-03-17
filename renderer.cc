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

#include "renderer.h"
#include "SDL.h"
#include "SDL_image.h"

using namespace std;

namespace foo {

RenderSystem::RenderSystem() {}

RenderSystem::RenderSystem(RenderSystem &&other) {
	swap(sdl_api_, other.sdl_api_);
	swap(sdl_image_api_, other.sdl_image_api_);
	swap(window_, other.window_);
	swap(renderer_, other.renderer_);
	swap(textures_, other.textures_);
	swap(repeated_textures_, other.repeated_textures_);
}

RenderSystem::~RenderSystem() {}

RenderSystem& RenderSystem::operator=(RenderSystem &&other) {
	if (this != &other) {
		textures_.clear();
		repeated_textures_.clear();
		sdl_api_.Destroy();
		sdl_image_api_.Destroy();
		window_.reset();
		renderer_.reset();

		swap(sdl_api_, other.sdl_api_);
		swap(sdl_image_api_, other.sdl_image_api_);
		swap(window_, other.window_);
		swap(renderer_, other.renderer_);
		swap(textures_, other.textures_);
		swap(repeated_textures_, other.repeated_textures_);
	}

	return *this;
}

void RenderSystem::Initialize() {
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

	SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Initializing RenderSystem...\n");

	sdl_api_.Create(SDL_INIT_VIDEO);
	sdl_image_api_.Create(IMG_INIT_PNG);

	SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "RenderSystem initialized.\n");
}

void RenderSystem::ProcessScene(
		const Scene &scene) {
	SDL_LogInfo(
		SDL_LOG_CATEGORY_RENDER,
		"RenderSystem: processing scene...\n");
	textures_.clear();
	repeated_textures_.clear();

	if (window_) {
		SDL_LogInfo(
			SDL_LOG_CATEGORY_RENDER,
			"Updating window...\n");

		SDL_SetWindowTitle(window_.get(), scene.title.c_str());
		SDL_SetWindowSize(window_.get(), scene.width, scene.height);
		SDL_SetWindowPosition(
			window_.get(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED);
	} else {
		SDL_LogInfo(
			SDL_LOG_CATEGORY_RENDER,
			"Creating window...\n");

		window_ = WindowPtr(SDL_CreateWindow(
			scene.title.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			scene.width,
			scene.height,
			0));
		if (!window_) {
			auto error_message = SDL_GetError();
			SDL_LogError(
				SDL_LOG_CATEGORY_RENDER,
				"Failed to create winow: %s\n",
				error_message);
			throw runtime_error(error_message);
		}

		SDL_LogInfo(
			SDL_LOG_CATEGORY_RENDER,
			"Creating renderer...\n");

		renderer_ = RendererPtr(SDL_CreateRenderer(
			window_.get(),
			-1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
		if (!renderer_) {
			auto error_message = SDL_GetError();
			SDL_LogError(
				SDL_LOG_CATEGORY_RENDER,
				"Failed to create renderer: %s\n",
				error_message);
			throw runtime_error(error_message);
		}
	}

	for (const auto &scene_object: scene.objects) {
		switch (scene_object.type) {
		case kSceneObjectTexture:
			textures_.emplace_back(
				ProcessTextureNode(scene_object));
			break;

		case kSceneObjectRepeatedTexture:
			repeated_textures_.emplace_back(
				ProcessRepeteadTextureNode(scene_object));
			break;

		default:
			SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "skipping node\n");
			break;
		}
	}
}

void
RenderSystem::ProcessSceneNodeCommon(
		TextureNode &node,
	    const SceneObject &scene_object) const {
	SDL_LogInfo(
		SDL_LOG_CATEGORY_RENDER,
		"Loading %s...\n",
		scene_object.path.c_str());
	SurfacePtr cpu_mem(IMG_Load(scene_object.path.c_str()));
	if (!cpu_mem) {
		auto error_message = IMG_GetError();
		SDL_LogError(
			SDL_LOG_CATEGORY_RENDER,
			"Failed to load image: %s\n",
			error_message);
		throw runtime_error(error_message);
	}

	node.destination.x = scene_object.x;
	node.destination.y = scene_object.y;
	node.destination.w = cpu_mem->w;
	node.destination.h = cpu_mem->h;
	node.texture = TexturePtr(SDL_CreateTextureFromSurface(
		renderer_.get(), cpu_mem.get()));
	if (!node.texture) {
		auto error_message = SDL_GetError();
		SDL_LogError(
			SDL_LOG_CATEGORY_RENDER,
			"Failed to create texture: %s\n",
			error_message);
		throw runtime_error(error_message);
	}
}

TextureNode
RenderSystem::ProcessTextureNode(
		const SceneObject &scene_object) const {
	SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Processing texture node...\n");

	TextureNode to;
	ProcessSceneNodeCommon(to, scene_object);
	return move(to);
}

RepeatedTextureNode
RenderSystem::ProcessRepeteadTextureNode(
		const SceneObject &scene_object) const {
	SDL_LogInfo(
		SDL_LOG_CATEGORY_RENDER,
		"Processing repeated texture node...\n");

	RepeatedTextureNode to;
	ProcessSceneNodeCommon(to, scene_object);
	to.repeat_x = scene_object.repeat_x;
	to.repeat_y = scene_object.repeat_y;
	return move(to);
}

void RenderSystem::Update(float /*elapsed_milliseconds*/) const {
	SDL_RenderClear(renderer_.get());

	for (const auto &item: textures_) {
		SDL_RenderCopy(
			renderer_.get(),
			item.texture.get(),
			nullptr,
			&item.destination);
	}

	for (const auto &item: repeated_textures_) {
		for (int y = 0; y < item.repeat_y; ++y) {
			SDL_Rect rt2 = item.destination;
			rt2.y = item.destination.y + item.destination.h * y;

			for (int x = 0; x < item.repeat_x; ++x) {
				rt2.x = item.destination.x + item.destination.w * x;
				SDL_RenderCopy(
					renderer_.get(),
					item.texture.get(),
					nullptr,
					&rt2);
			}
		}
	}

	SDL_RenderPresent(renderer_.get());
}

void RenderSystem::SdlApiTraits::Create(Uint32 flags) {
	SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Initializing SDL...\n");

	if (SDL_Init(flags) != 0) {
		auto error_message = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_RENDER,
			"Failed to initialize SDL: %s\n",
			error_message);
		throw runtime_error(error_message);
	}
}

void RenderSystem::SdlApiTraits::Destroy() {
	SDL_LogInfo(
		SDL_LOG_CATEGORY_RENDER,
		"Quitting SDL...");

	SDL_Quit();
}

void RenderSystem::SdlImageApiTraits::Create(int flags) {
	SDL_LogInfo(SDL_LOG_CATEGORY_RENDER, "Initializing SDL_image...\n");

	if (IMG_Init(flags) != flags) {
		auto error_message = IMG_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_RENDER,
			"Failed to initialize SDL_image: %s\n",
			error_message);
		throw runtime_error(error_message);
	}
}

void RenderSystem::SdlImageApiTraits::Destroy() {
	SDL_LogInfo(
		SDL_LOG_CATEGORY_RENDER,
		"Quitting SDL_image...");

	IMG_Quit();
}

} // namespace foo
