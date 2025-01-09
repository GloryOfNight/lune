#pragma once

#include "SDL3/SDL_surface.h"

#include <memory>

namespace lune
{
	namespace sdl
	{
		struct surfaceDeleter
		{
			void operator()(SDL_Surface* surface) const
			{
				SDL_DestroySurface(surface);
			}
		};


        using sharedSurface = std::shared_ptr<SDL_Surface>;
        using weakSurface = std::weak_ptr<SDL_Surface>;
        using uniqueSurface = std::unique_ptr<SDL_Surface, surfaceDeleter>;

		static sharedSurface makeSharedSurface(SDL_Surface* surface)
		{
			return sharedSurface(std::move(surface), surfaceDeleter());
		}

		static uniqueSurface makeUniqueSurface(SDL_Surface* surface)
		{
			return std::unique_ptr<SDL_Surface, surfaceDeleter>(std::move(surface));
		}

	} // namespace sdl
} // namespace lune