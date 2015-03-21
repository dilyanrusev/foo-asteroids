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

#ifndef FOO_ASTEROIDS_RENDERER_H_
#define FOO_ASTEROIDS_RENDERER_H_

#include "scene.h"
#include "handle.h"
#include "smart_pointers.h"
#include "SDL_rect.h"
#include <vector>
#include <utility>
#include <map>

struct SDL_Renderer;
struct SDL_Texture;

namespace foo {

class RenderSystem {
	struct SdlApiTraits {
		void Create(unsigned int flags);
		void Destroy();
	};
	struct SdlImageApiTraits {
		void Create(int flags);
		void Destroy();
	};
	struct SimpleRender {
		SDL_Rect destination;
		SDL_Rect clip;
	};
	struct RepeatingRender : public SimpleRender {
		int repeat_x;
		int repeat_y;
	};
	struct Node {
		TexturePtr texture;
		int width;
		int height;
		std::vector<SimpleRender> simple_renders;
		std::vector<RepeatingRender> repeating_renders;
	};

	Handle<SdlApiTraits> sdl_api_;
	Handle<SdlImageApiTraits> sdl_image_api_;
	WindowPtr window_;
	RendererPtr renderer_;
	std::vector<Node> nodes_;

public:
	RenderSystem();
	RenderSystem(const RenderSystem&) = delete;
	RenderSystem(RenderSystem&&) = delete;
	~RenderSystem();

	RenderSystem& operator=(const RenderSystem&) = delete;
	RenderSystem& operator=(RenderSystem&&) = delete;

	void Initialize();
	void ProcessScene(const Scene &scene);
	void Update(float elapsed_milliseconds) const;

private:
	void UpdateWindowFromScene(const Scene &scene);
	void CreateWindowFromScene(const Scene &scene);
	void CreateRendererFromScene(const Scene &scene);
	void UpdateNodesFromScene(const Scene &scene);

	Node LoadNode(const std::string &path) const;

	void FillTextureSimple(
		const Node &node,
		const SceneObject &scene_object,
		SimpleRender &render) const;

	void ProcessObjectForTextureReferences(
		const SceneObject &scene_object,
		const SceneTexture &scene_texture,
		Node &node) const;
};

} // namespace foo

#endif // FOO_ASTEROIDS_RENDERER_H_