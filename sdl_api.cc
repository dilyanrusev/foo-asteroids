#include "sdl_api.h"
#include "SDL.h"
#include <stdexcept>

namespace foo {

using namespace std;

void SdlApiTraits::Create(Uint32 flags) {
	if (SDL_Init(flags) != 0) {
		throw runtime_error(SDL_GetError());
	}
}

void SdlApiTraits::Destroy() {
	SDL_Quit();
}

} // namespace foo