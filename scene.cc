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

using namespace std;

namespace foo {

Scene LoadSceneFromFile(const char *file_name) {
	ifstream in_file(file_name);
	Json::Value value;
	in_file >> value;
	Scene scene;
	LoadScene(value, scene);
	return scene;
}

void LoadScene(const Json::Value &in, Scene &out) {
	out.id = in["id"].asString();
	out.title = in["title"].asString();
	out.width = in["width"].asInt();
	out.height = in["height"].asInt();

	const Json::Value &json_objects = in["objects"];
	for (Json::Value::ArrayIndex i = 0;
		i < json_objects.size();
		++i) {
		const Json::Value &json_object = json_objects[i];
		SceneObject ui_object;
		LoadSceneObject(json_object, ui_object);
		out.objects.push_back(ui_object);
	}
}

} // namespace foo