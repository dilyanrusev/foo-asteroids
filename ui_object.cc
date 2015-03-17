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

#include "ui_object.h"
#include "json/json.h"
#include <stdexcept>

using namespace std;

namespace foo {

void LoadSceneObject(const Json::Value &in, SceneObject &out) {
	const Json::Value &json_id = in["id"];
	if (!json_id.isNull()) {
		out.id = json_id.asString();
	}

	out.path = in["path"].asString();

	const Json::Value &json_pos = in["position"];
	out.x = json_pos[0].asInt();
	out.y = json_pos[1].asInt();

	const string& type_name = in["type"].asString();
	if (type_name == "texture") {
		out.type = kSceneObjectTexture;
		out.repeat_x = 1;
		out.repeat_y = 1;
	} else if (type_name == "repeated_texture") {
		out.type = kSceneObjectRepeatedTexture;

		const Json::Value &json_repeat = in["repeat"];
		out.repeat_x = json_repeat[0].asInt();
		out.repeat_y = json_repeat[1].asInt();
	} else {
		throw runtime_error(
			string("unknown scene object type: ")
			+ type_name);
	}
}

} // namespace foo