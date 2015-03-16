#ifndef FOO_ASTEROIDS_SDL_HANDLE_H_
#define FOO_ASTEROIDS_SDL_HANDLE_H_

#include "handle.h"

namespace foo {

struct SdlApiTraits {
	void Create(unsigned int flags);
	void Destroy();
};

using SdlApi = Handle<SdlApiTraits>;

}

#endif // FOO_ASTEROIDS_SDL_HANDLE_H_