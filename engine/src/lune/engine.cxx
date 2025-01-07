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

	uint32 major{}, minor{}, patch{};
	lune::getVersion(lune::getApplicationVersion(), major, minor, patch);
	LN_LOG(Info, Engine, "Application: \'{0}\', version {1}.{2}.{3}", lune::getApplicationName(), major, minor, patch);

	lune::getVersion(lune::getEngineVersion(), major, minor, patch);
	LN_LOG(Info, Engine, "Engine: \'{0}\', version {1}.{2}.{3}", lune::getEngineName(), major, minor, patch);

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

		auto vkSubsystem = vulkan_subsystem::get();

		for (uint32 view : mViews)
		{
			if (vkSubsystem->beginNextFrame(view))
			{
				auto [imageIndex, commandBuffer] = vkSubsystem->getFrameInfo(view);

				vkSubsystem->sumbitFrame(view);
			}
		}
	}
}

void lune::engine::createWindow(const std::string_view name, const uint32 width, const uint32 height, const uint32 flags)
{
	auto subsystem = vulkan_subsystem::get();
	if (subsystem)
	{
		auto newWindow = SDL_CreateWindow(name.data(), width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | flags);

		const uint32 newViewId = subsystem->createView(newWindow);
		if (newViewId != UINT32_MAX)
			mViews.emplace_back(newViewId);
	}
}
