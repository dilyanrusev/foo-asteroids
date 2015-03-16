#include <iostream>
#include "sdl_api.h"
#include "SDL.h"

using namespace std;
using namespace foo;

int main(int argc, char** argv) {
	SdlApi sdl_api(SDL_INIT_VIDEO);

	cout << "Hello, world!" << endl;

	return 0;
}