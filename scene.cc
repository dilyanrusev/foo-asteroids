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
#include <memory>
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
		objects_.clear();

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

	ProcessSpritesheets(prefix, in["spritesheets"]);
	ProcessTextures(prefix, in["textures"]);
	ProcessSceneObjects(prefix, in["objects"]);
}

void Scene::ProcessTextures(
		const string &prefix,
		const Json::Value &in) {
	SDL_LogInfo(
		SDL_LOG_CATEGORY_SYSTEM,
		"Processing scene textures...\n");

	textures_.clear();

	if (in.isNull() || !in.isArray()) {
		return;
	}

	for (Json::Value::ArrayIndex i = 0;
		i < in.size();
		++i) {
		const auto &json_object = in[i];

		SceneTexture texture;
		texture.id = json_object["id"].asString();
		texture.path = prefix + json_object["path"].asString();
		textures_.emplace_back(std::move(texture));
	}
}

void Scene::ProcessSpritesheets(
		const string &prefix,
		const Json::Value &in) {
	SDL_LogInfo(
		SDL_LOG_CATEGORY_SYSTEM,
		"Processing scene spritesheets...\n");

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
	objects_.clear();

	if (in.isNull() || !in.isArray()) {
		return;
	}

	for (Json::Value::ArrayIndex i = 0;
		i < in.size();
		++i) {
		const auto &json_object = in[i];

		SceneObject object;
		object.id = json_object["id"].asString();
		if (!object.id.size()) {
			SDL_LogWarn(
				SDL_LOG_CATEGORY_SYSTEM,
				"Empty object id: skipping\n");
			continue;
		}

		const auto &json_position = json_object["position"];
		if (json_position.isNull()
			|| !json_position.isArray()
			|| 2 != json_position.size()
			|| !json_position[0].isInt()
			|| !json_position[1].isInt()) {
			SDL_LogWarn(
				SDL_LOG_CATEGORY_SYSTEM,
				"Missing or malformatted position for %s: skipping\n",
				object.id.c_str());
			continue;
		}
		object.x = json_position[0].asInt();
		object.y = json_position[1].asInt();

		ProcessObjectComponents(prefix, json_object["components"], object);

		objects_.emplace_back(move(object));
	}
}

void Scene::ProcessObjectComponents(
		const string &prefix,
		const Json::Value &in,
		SceneObject &out) const {
	if (in.isNull()) {
		return;
	}

	for (Json::Value::ArrayIndex i = 0;
		i < in.size();
		++i) {
		const auto &json_object = in[i];

		const auto &json_type = json_object["type"];
		if (json_type.isNull() || !json_type.isString()) {
			SDL_LogWarn(
				SDL_LOG_CATEGORY_SYSTEM,
				"Missing or malformatted component type for"
				" %s: skipping\n",
				out.id.c_str());
			continue;
		}
		const auto &type = json_type.asString();

		if (type == "texture") {
			if (out.texture) {
				SDL_LogWarn(
					SDL_LOG_CATEGORY_SYSTEM,
					"Redefined texture component for %s: ignoring\n",
					out.id.c_str());
				continue;
			}

			out.texture = ProcessTextureComponent(
				out, json_object);
		} else if (type == "texture_repeat") {
			if (out.texture_repeat) {
				SDL_LogWarn(
					SDL_LOG_CATEGORY_SYSTEM,
					"Redefined texture_repeat component for %s: ignoring\n",
					out.id.c_str());
				continue;
			}

			out.texture_repeat = ProcessTextureRepeatComponent(
				out, json_object);
		} else {
			SDL_LogWarn(
				SDL_LOG_CATEGORY_SYSTEM,
				"Unknown component type %s for %s: ignoring\n",
				type.c_str(),
				out.id.c_str());
		}
	}
}

unique_ptr<SceneComponentTexture>
Scene::ProcessTextureComponent(
		const SceneObject &object,
		const Json::Value &in) const {
	const auto &json_texture_id = in["texture_id"];
	if (json_texture_id.isNull() || !json_texture_id.isString()) {
		SDL_LogWarn(
			SDL_LOG_CATEGORY_SYSTEM,
			"Missing texture_id for texture component "
			"in %s\n",
			object.id.c_str());
		return nullptr;
	}

	auto ptr = unique_ptr<SceneComponentTexture>(
		new SceneComponentTexture);
	ptr->texture_id = json_texture_id.asString();
	return move(ptr);
}

unique_ptr<SceneComponentTextureRepeat>
Scene::ProcessTextureRepeatComponent(
		const SceneObject &object,
		const Json::Value &in) const {
	const auto &json_repeat = in["repeat"];
	if (json_repeat.isNull()
		|| !json_repeat.isArray()
		|| json_repeat.size() != 2
		|| !json_repeat[0].isInt()
		|| !json_repeat[1].isInt()) {
		SDL_LogWarn(
			SDL_LOG_CATEGORY_SYSTEM,
			"Missing or malformated repeat texture_repeat "
			"comonent in %s\n",
			object.id.c_str());
		return nullptr;
	}

	auto ptr = unique_ptr<SceneComponentTextureRepeat>(
		new SceneComponentTextureRepeat);
	ptr->repeat_x = json_repeat[0].asInt();
	ptr->repeat_y = json_repeat[1].asInt();
	return move(ptr);
}

} // namespace foo