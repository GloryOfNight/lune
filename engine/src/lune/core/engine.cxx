#include "lune/core/engine.hxx"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "lune/core/log.hxx"
#include "lune/lune.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

static lune::Engine* gEngine{nullptr};

lune::Engine* lune::Engine::get()
{
	return gEngine;
}

bool lune::Engine::initialize(std::vector<std::string> args)
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

bool lune::Engine::wasInitialized() const
{
	return gEngine == this;
}

void lune::Engine::shutdown()
{
	for (auto& engineSubsystem : mSubsystems)
	{
		engineSubsystem->shutdown();
	}

	SDL_Quit();
}

void lune::Engine::run()
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

		for (auto& [sId, s] : mScenes)
		{
			s->update(0);
		}

		auto vkSubsystem = vulkan_subsystem::get();

		for (uint32 view : mViews)
		{
			if (vkSubsystem->beginNextFrame(view))
			{
				auto [imageIndex, commandBuffer] = vkSubsystem->getFrameInfo(view);

				for (auto& [sId, s] : mScenes)
				{
					s->render();
				}

				vkSubsystem->sumbitFrame(view);
			}
		}
	}
}

void lune::Engine::createWindow(const std::string_view name, const uint32 width, const uint32 height, const uint32 flags)
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

uint64 lune::Engine::addScene(std::unique_ptr<Scene> s)
{
	static uint64 sIdCounter = 0;
	if (s) [[likely]]
		return mScenes.emplace_back(std::pair<uint64, std::unique_ptr<Scene>>{++sIdCounter, std::move(s)}).first;
	return uint64();
}
