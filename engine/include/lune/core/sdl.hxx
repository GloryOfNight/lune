#pragma once

#include "SDL3/SDL.h"

#include <memory>
#include <vector>

namespace lune
{
	struct UniqueSDLSurfaceDeleter
	{
		void operator()(SDL_Surface* surface)
		{
			SDL_DestroySurface(surface);
		}
	};

	using UniqueSDLSurface = std::unique_ptr<SDL_Surface, UniqueSDLSurfaceDeleter>;
} // namespace lune