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

#ifndef FOO_ASTEROIDS_UI_OBJECT_H_
#define FOO_ASTEROIDS_UI_OBJECT_H_

#include <string>
#include "json/json-forwards.h"

namespace foo {

enum SceneObjectKind {
	kSceneObjectTexture,
	kSceneObjectRepeatedTexture,
	kCountSceneObjects
};

struct SceneObject {
	SceneObjectKind type;
	std::string id;
	std::string path;
	int x;
	int y;
	int repeat_x;
	int repeat_y;
};

void LoadSceneObject(const Json::Value &in, SceneObject &out);

} // namespace foo

#endif // FOO_ASTEROIDS_UI_OBJECT_H_