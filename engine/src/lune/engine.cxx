#include "engine.hxx"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "renderer/vulkan/vulkan_subsystem.hxx"

#include "log.hxx"
#include "lune.hxx"

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

	LN_LOG(Info, Engine, "Engine initilized. {0} - {1}.{2}.{3}", getEngineName(), getEngineVersionMajor(), getEngineVersionMinor(), getEngineVersionPatch());

	mArgs = std::move(args);

	addSubsystem<vulkan_subsystem>();

	for (auto it = mSubsystems.begin(); it != mSubsystems.end(); it++)
	{
		if (it->get()->allowInitialize())
		{
			it->get()->initialize();
		}
		else
		{
			auto eraseIt = it;
			--it;
			mSubsystems.erase(eraseIt);
		}
	}

	return true;
}

bool lune::engine::wasInitialized() const
{
	return gEngine == this;
}

void lune::engine::shutdown()
{
	for (auto& engineSubsystem : mSubsystems)
	{
		engineSubsystem->shutdown();
	}

	SDL_Quit();
}

void lune::engine::run()
{
	while (true)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				return;
			}
		}
	}
}

void lune::engine::createWindow(const std::string_view name, const uint32 width, const uint32 height, const uint32 flags)
{
	auto subsystem = vulkan_subsystem::get();
	if (subsystem)
	{
		auto newWindow = SDL_CreateWindow(name.data(), width, height, SDL_WINDOW_VULKAN | flags);
		subsystem->createView(newWindow);
	}
}
