#pragma once

#include "SDL3/SDL_surface.h"

#include <memory>

namespace lune
{
	namespace sdl
	{
		struct SurfaceDeleter
		{
			void operator()(SDL_Surface* surface) const
			{
				SDL_DestroySurface(surface);
			}
		};

		using SharedSurface = std::shared_ptr<SDL_Surface>;
		using WeakSurface = std::weak_ptr<SDL_Surface>;
		using UniqueSurface = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

		static SharedSurface makeSharedSurface(SDL_Surface* surface)
		{
			return SharedSurface(std::move(surface), SurfaceDeleter());
		}

		static UniqueSurface makeUniqueSurface(SDL_Surface* surface)
		{
			return std::unique_ptr<SDL_Surface, SurfaceDeleter>(std::move(surface));
		}

	} // namespace sdl
} // namespace lune