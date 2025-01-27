#pragma once

#include "SDL3/SDL.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3_image/SDL_image.h"

#include <lune/core/log.hxx>
#include <memory>
#include <string>

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

	static UniqueSDLSurface loadTextureImage(const std::string_view& path)
	{
		auto newSurface = UniqueSDLSurface(IMG_Load(path.data()));
		if (newSurface)
		{
			if (newSurface->format == SDL_PIXELFORMAT_RGB24)
			{
				return UniqueSDLSurface(SDL_ConvertSurface(newSurface.get(), SDL_PIXELFORMAT_RGBA32));
			}
			if (newSurface->format == SDL_PIXELFORMAT_RGBA32 || newSurface->format == SDL_PIXELFORMAT_BGRA8888)
			{
				return newSurface;
			}
			else
			{
				LN_LOG(Fatal, Lune::LoadTextureImage, "Unsupported texture format!");
				return nullptr;
			}
		}
		return nullptr;
	}
} // namespace lune