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
#include "tinyxml2.h"
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
		spritesheets_.clear();
		texture_objects_.clear();
		repeated_texture_objects_.clear();

		swap(*this, other);
	}
	return *this;
}

void Scene::LoadFromFile(const char *file_name) {
	SDL_LogInfo(
		SDL_LOG_CATEGORY_SYSTEM,
		"Loading scene from %s...\n",
		file_name);

	string prefix(file_name);
	auto last_separator = prefix.find_last_of('\\');
	if (string::npos == last_separator) {
		last_separator = prefix.find_last_of('/');
	}
	if (string::npos != last_separator) {
		prefix.erase(last_separator + 1, string::npos);
	}

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

	const auto &json_spritesheets = in["spritesheets"];
	ProcessSpritesheets(prefix, json_spritesheets);

	const auto &json_objects = in["objects"];
	ProcessSceneObjects(prefix, json_objects);
}

void Scene::ProcessSpritesheets(
		const string &prefix,
		const Json::Value &in) {
	spritesheets_.clear();

	if (in.isNull() || !in.isArray()) {
		return;
	}

	for (Json::Value::ArrayIndex i = 0;
		i < in.size();
		++i) {
		const auto &json_object = in[i];

		SceneSpritesheet sheet;
		sheet.id = json_object["id"].asString();
		sheet.path = prefix + json_object["path"].asString();
		ProcessTextureAtlasXml(prefix, sheet);
	}
}

void Scene::ProcessTextureAtlasXml(
		const string &prefix,
		SceneSpritesheet &out) const {
	SDL_LogInfo(
		SDL_LOG_CATEGORY_SYSTEM,
		"Processing XML texture atlas %s...\n",
		out.path.c_str());

	using namespace tinyxml2;

	out.regions.clear();
	XMLDocument doc;
	XMLError error;

	error = doc.LoadFile(out.path.c_str());
	if (error != XML_NO_ERROR) {
		SDL_LogError(
			SDL_LOG_CATEGORY_SYSTEM,
			"Failed to load atlas %s: %d\n",
			out.path.c_str(),
			error);
		throw runtime_error("Failed to load atlas file");
	}

	XMLElement *root = doc.RootElement();
	if (!root) {
		throw runtime_error("Atlas file has no root XML element");
	}

	const char *raw_path = root->Attribute("imagePath");
	if (!raw_path) {
		throw runtime_error("Failed to get TextureAtlas.imagePath");
	}

	out.image_path = prefix + string(raw_path);

	for (XMLElement *current = root->FirstChildElement("SubTexture");
			current;
			current = current->NextSiblingElement("SubTexture")) {
		SceneSceneSpritesheetRegion region;

		const char *raw_name = current->Attribute("name");
		if (!raw_name) {
			throw runtime_error("Failed to get SubTexture.name");
		}
		region.name = raw_name;

		if (current->QueryAttribute("x", &region.x)
				!= XML_NO_ERROR) {
			SDL_LogError(
				SDL_LOG_CATEGORY_SYSTEM,
				"Failed on element %s\n",
				raw_name);
			throw runtime_error("Failed to get SubTexture.x");
		}

		if (current->QueryAttribute("y", &region.y)
				!= XML_NO_ERROR) {
			SDL_LogError(
				SDL_LOG_CATEGORY_SYSTEM,
				"Failed on element %s\n",
				raw_name);
			throw runtime_error("Failed to get SubTexture.y");
		}

		if (current->QueryAttribute("width", &region.width)
				!= XML_NO_ERROR) {
			SDL_LogError(
				SDL_LOG_CATEGORY_SYSTEM,
				"Failed on element %s\n",
				raw_name);
			throw runtime_error("Failed to get SubTexture.width");
		}

		if (current->QueryAttribute("height", &region.height)
				!= XML_NO_ERROR) {
			SDL_LogError(
				SDL_LOG_CATEGORY_SYSTEM,
				"Failed on element %s\n",
				raw_name);
			throw runtime_error("Failed to get SubTexture.height");
		}

		out.regions.emplace_back(move(region));
	}

	SDL_LogInfo(
		SDL_LOG_CATEGORY_SYSTEM,
		"Processed %lu SubTexture-s.\n",
		static_cast<unsigned long>(out.regions.size()));
}

void Scene::ProcessSceneObjects(
		const string &prefix,
		const Json::Value &in) {
	SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Processing scene objects...\n");
	texture_objects_.clear();
	repeated_texture_objects_.clear();

	if (in.isNull() || !in.isArray()) {
		return;
	}

	for (Json::Value::ArrayIndex i = 0;
		i < in.size();
		++i) {
		const auto &json_object = in[i];
		const auto &type = json_object["type"].asString();

		if (type == "texture") {
			texture_objects_.emplace_back(
				LoadTexture(prefix, json_object));
		} else if (type == "repeated_texture") {
			repeated_texture_objects_.emplace_back(
				LoadRepeatedTexture(prefix, json_object));
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
		const string &prefix,
		const Json::Value &in,
		SceneObjectTexture &out) const {
	const Json::Value &json_id = in["id"];
	if (!json_id.isNull()) {
		out.id = json_id.asString();
	}

	out.path = prefix + in["path"].asString();

	const auto &json_pos = in["position"];
	out.x = json_pos[0].asInt();
	out.y = json_pos[1].asInt();
}

SceneObjectTexture
Scene::LoadTexture(
		const string &prefix,
		const Json::Value &in) const {
	SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Loading texture...\n");

	SceneObjectTexture out;

	LoadTextureCommon(prefix, in, out);

	return move(out);
}

SceneObjectRepeatedTexture
Scene::LoadRepeatedTexture(
		const string &prefix,
		const Json::Value &in) const {
	SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "Loading repeated texture...\n");

	SceneObjectRepeatedTexture out;

	LoadTextureCommon(prefix, in, out);

	const auto &json_repeat = in["repeat"];
	out.repeat_x = json_repeat[0].asInt();
	out.repeat_y = json_repeat[1].asInt();

	return move(out);
}

} // namespace foo