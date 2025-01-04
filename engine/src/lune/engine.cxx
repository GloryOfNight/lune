#include "engine.hxx"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

static lune::engine* gEngine{nullptr};

lune::engine* lune::engine::get()
{
	return gEngine;
}

bool lune::engine::initialize(std::vector<std::string> args)
{
	if (gEngine != nullptr)
		return false;

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
		return false;

	if (!SDL_Vulkan_LoadLibrary(NULL))
		return false;

	mArgs = std::move(args);

	return true;
}

void lune::engine::shutdown()
{
	SDL_Quit();
}

void lune::engine::run()
{
}