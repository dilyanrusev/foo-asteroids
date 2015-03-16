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

#include "helpers.h"
#include "SDL.h"
#include "SDL_image.h"
#include <stdexcept>

using namespace std;

namespace foo {

TexturePtr LoadTexture(SDL_Renderer *renderer, const char *image_path) {
	SurfacePtr surface(IMG_Load(image_path));
	if (!surface) {
		throw runtime_error(IMG_GetError());
	}

	TexturePtr texture(SDL_CreateTextureFromSurface(
		renderer, surface.get()));
	if (!texture) {
		throw runtime_error(SDL_GetError());
	}

	return move(texture);
}

} // namespace foo