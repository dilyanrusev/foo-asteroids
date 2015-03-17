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
	swap(textures_, other.textures_);
	swap(repeated_textures_, other.repeated_textures_);
}

RenderSystem::~RenderSystem() {}

RenderSystem& RenderSystem::operator=(RenderSystem &&other) {
	if (this != &other) {
		textures_.clear();
		repeated_textures_.clear();

		swap(textures_, other.textures_);
		swap(repeated_textures_, other.repeated_textures_);
	}
	return *this;
}

void RenderSystem::ProcessScene(
		SDL_Renderer *renderer,
		const Scene &scene) {
	textures_.clear();
	repeated_textures_.clear();

	for (const auto &scene_object: scene.objects) {
		switch (scene_object.type) {
		case kSceneObjectTexture:
			textures_.emplace_back(
				ProcessTextureNode(renderer, scene_object));
			break;

		case kSceneObjectRepeatedTexture:
			repeated_textures_.emplace_back(
				ProcessRepeteadTextureNode(renderer, scene_object));
			break;

		default:
			break;
		}
	}
}

void
RenderSystem::ProcessSceneNodeCommon(
		SDL_Renderer *renderer,
		TextureNode &node,
	    const UiObject &scene_object) {
	SurfacePtr cpu_mem(IMG_Load(scene_object.path.c_str()));
	if (!cpu_mem) {
		throw runtime_error(IMG_GetError());
	}

	node.destination.x = scene_object.x;
	node.destination.y = scene_object.y;
	node.destination.w = cpu_mem->w;
	node.destination.h = cpu_mem->h;
	node.texture = TexturePtr(SDL_CreateTextureFromSurface(
		renderer, cpu_mem.get()));
	if (!node.texture) {
		throw runtime_error(SDL_GetError());
	}
}

TextureNode
RenderSystem::ProcessTextureNode(
		SDL_Renderer *renderer,
		const UiObject &scene_object) {
	TextureNode to;
	ProcessSceneNodeCommon(renderer, to, scene_object);
	return move(to);
}

RepeatedTextureNode
RenderSystem::ProcessRepeteadTextureNode(
		SDL_Renderer *renderer,
		const UiObject &scene_object) {
	RepeatedTextureNode to;
	ProcessSceneNodeCommon(renderer, to, scene_object);
	to.repeat_x = scene_object.repeat_x;
	to.repeat_y = scene_object.repeat_y;
	return move(to);
}

void RenderSystem::Render(SDL_Renderer *renderer) const {
	for (const auto &item: textures_) {
		SDL_RenderCopy(
			renderer,
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
					renderer,
					item.texture.get(),
					nullptr,
					&rt2);
			}
		}
	}
}

} // namespace foo
