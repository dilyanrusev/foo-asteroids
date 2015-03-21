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
#include <algorithm>

using namespace std;

namespace foo {

RenderSystem::RenderSystem() {}

RenderSystem::~RenderSystem() {}

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
	if (window_) {
		UpdateWindowFromScene(scene);
	} else {
		CreateWindowFromScene(scene);
		CreateRendererFromScene(scene);
	}

	UpdateNodesFromScene(scene);
}

void RenderSystem::UpdateNodesFromScene(const Scene &scene) {
	nodes_.clear();

    for (const auto &scene_texture: scene.textures()) {
        Node node = LoadNode(scene_texture.path);
        for (const auto &scene_object: scene.objects()) {
            ProcessObjectForTextureReferences(
                scene_object, scene_texture, node);
        }

        if (!node.repeating_renders.empty()
                || !node.simple_renders.empty()) {
            SDL_LogInfo(SDL_LOG_CATEGORY_RENDER,
                    "Adding node for %s\n",
                    scene_texture.id.c_str());
            nodes_.emplace_back(move(node));
        }
    }

    for (const auto &scene_spritesheet: scene.spritesheets()) {
        Node node = LoadNode(scene_spritesheet.image_path);
        string id_start = scene_spritesheet.id + ":";
        for (const auto &scene_object: scene.objects()) {
            ProcessObjectForSpritesheetReferences(
                scene_object, scene_spritesheet, id_start, node);
        }

        if (!node.repeating_renders.empty()
                || !node.simple_renders.empty()) {
            SDL_LogInfo(SDL_LOG_CATEGORY_RENDER,
                    "Adding node for %s\n",
                    scene_spritesheet.id.c_str());
            nodes_.emplace_back(move(node));
        }
    }
}

void RenderSystem::ProcessObjectForSpritesheetReferences(
        const SceneObject &scene_object,
        const SceneSpritesheet &scene_spritesheet,
        const string &id_start,
        Node &node) const {
    if (!scene_object.texture
        || scene_object.texture->texture_id.find(id_start)
            != 0) {
        return;
    }

    string subtexture =
        scene_object.texture->texture_id.substr(id_start.size());

    auto iter = find_if(
        begin(scene_spritesheet.regions),
        end(scene_spritesheet.regions),
        [&subtexture](const SceneSceneSpritesheetRegion &r)
        { return r.name == subtexture; });

    if (end(scene_spritesheet.regions) == iter) {
        SDL_LogWarn(SDL_LOG_CATEGORY_RENDER,
            "%s: texture_id=%s. Found spritesheet %s, but"
            " no sub-region matches %s\n",
            scene_object.id.c_str(),
            scene_object.texture->texture_id.c_str(),
            scene_spritesheet.id.c_str(),
            subtexture.c_str());
        return;
    }

    SimpleRender render;
    render.destination.x = scene_object.x;
    render.destination.y = scene_object.y;
    render.destination.w = iter->width;
    render.destination.h = iter->height;
    render.clip.x = iter->x;
    render.clip.y = iter->y;
    render.clip.w = iter->width;
    render.clip.h = iter->width;

    SDL_LogInfo(SDL_LOG_CATEGORY_RENDER,
        "Creating sprite simple render\n");
    node.simple_renders.emplace_back(move(render));
}

void RenderSystem::ProcessObjectForTextureReferences(
        const SceneObject &scene_object,
        const SceneTexture &scene_texture,
        Node &node) const {
    if (!scene_object.texture
            || scene_object.texture->texture_id
                != scene_texture.id) {
        return;
    }

    if (scene_object.texture_repeat) {
        RepeatingRender render;
        FillTextureSimple(node, scene_object, render);
        render.repeat_x = scene_object.texture_repeat->repeat_x;
        render.repeat_y = scene_object.texture_repeat->repeat_y;

        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER,
            "Creating texture repeating render\n");
        node.repeating_renders.emplace_back(move(render));
    } else {
        SimpleRender render;
        FillTextureSimple(node, scene_object, render);

        SDL_LogInfo(SDL_LOG_CATEGORY_RENDER,
            "Creating texture simple render\n");
        node.simple_renders.emplace_back(move(render));
    }
}

void RenderSystem::FillTextureSimple(
        const Node &node,
        const SceneObject &scene_object,
        SimpleRender &render) const {
    render.destination.x = scene_object.x;
    render.destination.y = scene_object.y;
    render.destination.w = node.width;
    render.destination.h = node.height;
    render.clip.x = 0;
    render.clip.y = 0;
    render.clip.w = node.width;
    render.clip.h = node.height;
}

void RenderSystem::CreateRendererFromScene(const Scene &scene) {
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

void RenderSystem::CreateWindowFromScene(const Scene &scene) {
	SDL_LogInfo(
		SDL_LOG_CATEGORY_RENDER,
		"Creating window...\n");

	window_ = WindowPtr(SDL_CreateWindow(
		scene.title().c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		scene.width(),
		scene.height(),
		0));
	if (!window_) {
		auto error_message = SDL_GetError();
		SDL_LogError(
			SDL_LOG_CATEGORY_RENDER,
			"Failed to create winow: %s\n",
			error_message);
		throw runtime_error(error_message);
	}
}

void RenderSystem::UpdateWindowFromScene(const Scene &scene) {
	SDL_LogInfo(
		SDL_LOG_CATEGORY_RENDER,
		"Updating window...\n");

	SDL_SetWindowTitle(window_.get(), scene.title().c_str());
	SDL_SetWindowSize(window_.get(), scene.width(), scene.height());
	SDL_SetWindowPosition(
		window_.get(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED);
}

RenderSystem::Node RenderSystem::LoadNode(const std::string &path) const {
    SDL_LogInfo(
        SDL_LOG_CATEGORY_RENDER,
        "Loading %s...\n",
        path.c_str());

    SurfacePtr cpu_mem(IMG_Load(path.c_str()));
    if (!cpu_mem) {
        auto error_message = IMG_GetError();
        SDL_LogError(
            SDL_LOG_CATEGORY_RENDER,
            "Failed to load image: %s\n",
            error_message);
        throw runtime_error(error_message);
    }

    Node node;
    node.width = cpu_mem->w;
    node.height = cpu_mem->h;
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

    return move(node);
}

void RenderSystem::Update(float /*elapsed_milliseconds*/) const {
	SDL_RenderClear(renderer_.get());

	for (const auto &node: nodes_) {
		for (const auto &simple: node.simple_renders) {
			SDL_RenderCopy(
				renderer_.get(),
				node.texture.get(),
				&simple.clip,
				&simple.destination);
		}

		for (const auto &repeating: node.repeating_renders) {
			for (int y = 0; y < repeating.repeat_y; ++y) {
				SDL_Rect rt2 = repeating.destination;
				rt2.y =
					repeating.destination.y
					+ repeating.destination.h * y;

				for (int x = 0; x < repeating.repeat_x; ++x) {
					rt2.x =
						repeating.destination.x
						+ repeating.destination.w * x;
					SDL_RenderCopy(
						renderer_.get(),
						node.texture.get(),
						&repeating.clip,
						&rt2);
				}
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
