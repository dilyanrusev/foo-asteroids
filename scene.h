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
#include <memory>
#include "json/json-forwards.h"

namespace foo {

struct SceneSceneSpritesheetRegion {
	std::string name;
	int x;
	int y;
	int width;
	int height;
};

struct SceneSpritesheet {
	std::string id;
	std::string path;
	std::string image_path;
	std::vector<SceneSceneSpritesheetRegion> regions;
};

struct SceneTexture {
	std::string id;
	std::string path;
};

struct SceneComponentTexture {
	std::string texture_id;
};

struct SceneComponentTextureRepeat {
	int repeat_x;
	int repeat_y;
};

struct SceneObject {
	std::string id;
	int x;
	int y;
	std::unique_ptr<SceneComponentTexture> texture;
	std::unique_ptr<SceneComponentTextureRepeat> texture_repeat;
};

class Scene {
	std::string id_;
	std::string title_;
	int width_;
	int height_;
	std::vector<SceneTexture> textures_;
	std::vector<SceneSpritesheet> spritesheets_;
	std::vector<SceneObject> objects_;

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
		swap(lhs.spritesheets_, rhs.spritesheets_);
		swap(lhs.objects_, rhs.objects_);
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

	inline const std::vector<SceneTexture>&
	textures() const { return textures_; }

	inline const std::vector<SceneSpritesheet>&
	spritesheets() const { return spritesheets_; }

	inline const std::vector<SceneObject>&
	objects() const { return objects_; }

private:
	void
	ProcessSceneObjects(
		const std::string &prefix,
		const Json::Value &in);

	void
	ProcessObjectComponents(
		const std::string &prefix,
		const Json::Value &in,
		SceneObject &out) const;

	std::unique_ptr<SceneComponentTexture>
	ProcessTextureComponent(
		const SceneObject &object,
		const Json::Value &in) const;

	std::unique_ptr<SceneComponentTextureRepeat>
	ProcessTextureRepeatComponent(
		const SceneObject &object,
		const Json::Value &in) const;

	void
	ProcessSpritesheets(
		const std::string &prefix,
		const Json::Value &in);

	void
	ProcessTextureAtlasXml(
		const std::string &prefix,
		SceneSpritesheet &out) const;

	void ProcessTextures(
		const std::string &prefix,
		const Json::Value &in);
};

} // namespace foo

#endif // FOO_ASTEROIDS_SCENE_H_