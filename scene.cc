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

#include "scene.h"
#include "json/json.h"
#include <fstream>
#include "SDL_log.h"

using namespace std;

namespace foo {

Scene::Scene() {}

Scene::Scene(Scene &&other) {
	swap(*this, other);
}

Scene::~Scene() {}

Scene& Scene::operator=(Scene &&other) {
	if (this != &other) {
		id_.clear();
		title_.clear();
		textures_.clear();
		repeated_textures_.clear();

		swap(*this, other);
	}
	return *this;
}

void Scene::LoadFromFile(const char *file_name) {
	SDL_LogInfo(
		SDL_LOG_CATEGORY_SYSTEM,
		"Loading scene from %s...\n",
		file_name);

	string path(file_name);
	size_t last_separator = path.find_last_of('\\');
	if (string::npos == last_separator) {
		last_separator = path.find_last_of('/');
	}
	if (string::npos != last_separator) {
		path.erase(last_separator + 1, string::npos);
	}

	textures_.clear();
	repeated_textures_.clear();

	ifstream in_file(file_name);
	Json::Value in;
	in_file >> in;

	const Json::Value &json_id = in["id"];
	if (!json_id.isNull()) {
		id_ = json_id.asString();
	}

	title_ = in["title"].asString();
	width_ = in["width"].asInt();
	height_ = in["height"].asInt();

	const Json::Value &json_objects = in["objects"];
	for (Json::Value::ArrayIndex i = 0;
		i < json_objects.size();
		++i) {
		const Json::Value &json_object = json_objects[i];
		const string &type = json_object["type"].asString();

		if (type == "texture") {
			textures_.emplace_back(
				LoadTexture(path.c_str(), json_object));
		} else if (type == "repeated_texture") {
			repeated_textures_.emplace_back(
				LoadRepeatedTexture(path.c_str(), json_object));
		}
		else {
			SDL_LogWarn(
				SDL_LOG_CATEGORY_SYSTEM,
				"Unrecognized scene object type: %s\n",
				type.c_str());
			continue;
		}
	}
}

void Scene::LoadTextureCommon(
		const char *scene_file_name,
		const Json::Value &in,
		SceneObjectTexture &out) const {
	const Json::Value &json_id = in["id"];
	if (!json_id.isNull()) {
		out.id = json_id.asString();
	}

	out.path = string(scene_file_name) + in["path"].asString();

	const Json::Value &json_pos = in["position"];
	out.x = json_pos[0].asInt();
	out.y = json_pos[1].asInt();
}

SceneObjectTexture
Scene::LoadTexture(
		const char *scene_file_name,
		const Json::Value &in) const {
	SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Loading texture...\n");

	SceneObjectTexture out;

	LoadTextureCommon(scene_file_name, in, out);

	return move(out);
}

SceneObjectRepeatedTexture
Scene::LoadRepeatedTexture(
		const char *scene_file_name,
		const Json::Value &in) const {
	SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Loading repeated texture...\n");

	SceneObjectRepeatedTexture out;

	LoadTextureCommon(scene_file_name, in, out);

	const Json::Value &json_repeat = in["repeat"];
	out.repeat_x = json_repeat[0].asInt();
	out.repeat_y = json_repeat[1].asInt();

	return move(out);
}

} // namespace foo