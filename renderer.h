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
#include "smart_pointers.h"
#include "SDL_rect.h"
#include <vector>
#include <utility>

struct SDL_Renderer;

namespace foo {

struct TextureNode {
	SDL_Rect destination;
	TexturePtr texture;
};

struct RepeatedTextureNode : public TextureNode {
	int repeat_x;
	int repeat_y;
};

class RenderSystem {
	std::vector<TextureNode> textures_;
	std::vector<RepeatedTextureNode> repeated_textures_;

public:
	RenderSystem();
	RenderSystem(const RenderSystem&) = delete;
	RenderSystem(RenderSystem &&other);
	~RenderSystem();

	RenderSystem& operator=(const RenderSystem&) = delete;
	RenderSystem& operator=(RenderSystem &&other);
	friend void swap(RenderSystem &lhs, RenderSystem &rhs) {
		using std::swap;
		swap(lhs.textures_, rhs.textures_);
		swap(lhs.repeated_textures_, rhs.repeated_textures_);
	}

	void ProcessScene(SDL_Renderer *renderer, const Scene &scene);
	void Render(SDL_Renderer *renderer) const;

private:
	static void
	ProcessSceneNodeCommon(
		SDL_Renderer *renderer,
		TextureNode &node,
	    const UiObject &scene_object);

	static TextureNode
	ProcessTextureNode(
		SDL_Renderer *renderer,
		const UiObject &scene_object);

	static RepeatedTextureNode
	ProcessRepeteadTextureNode(
		SDL_Renderer *renderer,
		const UiObject &scene_object);
};

} // namespace foo

#endif // FOO_ASTEROIDS_RENDERER_H_