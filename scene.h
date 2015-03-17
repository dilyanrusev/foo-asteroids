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

#ifndef FOO_ASTEROIDS_SCENE_H_
#define FOO_ASTEROIDS_SCENE_H_

#include <string>
#include <vector>
#include <utility>
#include "json/json-forwards.h"

struct SDL_Renderer;

namespace foo {

struct SceneObjectTexture {
	std::string id;
	std::string path;
	int x;
	int y;
};

struct SceneObjectRepeatedTexture : public SceneObjectTexture {
	int repeat_x;
	int repeat_y;
};

class Scene {
	std::string id_;
	std::string title_;
	int width_;
	int height_;
	std::vector<SceneObjectTexture> textures_;
	std::vector<SceneObjectRepeatedTexture> repeated_textures_;

public:
	Scene();
	Scene(const Scene&) = delete;
	Scene(Scene &&other);
	~Scene();

	Scene& operator=(const Scene&) = delete;
	Scene& operator=(Scene &&other);
	friend void swap(Scene &lhs, Scene &rhs) {
		using std::swap;

		swap(lhs.id_, rhs.id_);
		swap(lhs.title_, rhs.title_);
		swap(lhs.width_, rhs.width_);
		swap(lhs.height_, rhs.height_);
		swap(lhs.textures_, rhs.textures_);
		swap(lhs.repeated_textures_, rhs.repeated_textures_);
	}

	void
	LoadFromFile(const char *file_name);

	inline const std::string&
	id() const { return id_; }

	inline const std::string&
	title() const { return title_; }

	inline int
	width() const { return width_; }

	inline int
	height() const { return height_; }

	inline const std::vector<SceneObjectTexture>&
	textures() const { return textures_; }

	inline const std::vector<SceneObjectRepeatedTexture>&
	repeated_textures() const { return repeated_textures_; }

private:
	void
	LoadTextureCommon(
		const char *scene_file_name,
		const Json::Value &in,
		SceneObjectTexture &out) const;

	SceneObjectTexture
	LoadTexture(
		const char *scene_file_name,
		const Json::Value &in) const;

	SceneObjectRepeatedTexture
	LoadRepeatedTexture(
		const char *scene_file_name,
		const Json::Value &in) const;
};

} // namespace foo

#endif // FOO_ASTEROIDS_SCENE_H_