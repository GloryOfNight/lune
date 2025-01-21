#include "lune/core/engine.hxx"

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "SDL3_ttf/SDL_ttf.h"
#include "lune/core/assets.hxx"
#include "lune/core/event_subsystem.hxx"
#include "lune/core/log.hxx"
#include "lune/core/timer_subsystem.hxx"
#include "lune/lune.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

#include <filesystem>

#if HAVE_SHADER_COMPILER_TOOL
#include "shader_compiler_tool/shader_compiler.hxx"
#endif

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

	if (!TTF_Init())
		return false;

#if HAVE_SHADER_COMPILER_TOOL
	const ShaderCompiler::Result sctRes = ShaderCompiler::compileShaders(*EngineShadersSourceDir(), *EngineShadersBinaryDir(), ShaderCompiler::TargetVkVersion::Vulkan13);
	if (sctRes != ShaderCompiler::Result::Success)
		return false;
#endif

	uint32 major{}, minor{}, patch{};
	lune::getVersion(lune::getApplicationVersion(), major, minor, patch);
	LN_LOG(Info, Engine, "Application: \'{0}\', version {1}.{2}.{3}", lune::getApplicationName(), major, minor, patch);

	lune::getVersion(lune::getEngineVersion(), major, minor, patch);
	LN_LOG(Info, Engine, "Engine: \'{0}\', version {1}.{2}.{3}", lune::getEngineName(), major, minor, patch);

	mArgs = std::move(args);
	gEngine = this;

	auto eventSubsystem = addSubsystem<EventSubsystem>();
	eventSubsystem->addEventBindingMem(SDL_EVENT_QUIT, this, &Engine::onSdlQuitEvent);
	eventSubsystem->addEventBindingMem(SDL_EVENT_WINDOW_CLOSE_REQUESTED, this, &Engine::onSdlWindowCloseEvent);

	addSubsystem<TimerSubsystem>();
	addSubsystem<VulkanSubsystem>();

	mInitialized = true;
	return true;
}

bool lune::Engine::wasInitialized() const
{
	return mInitialized;
}

void lune::Engine::shutdown()
{
	mScenes.clear();
	mSubsystems.clear();
	mViews.clear();
	mArgs.clear();

	if (gEngine == this)
		gEngine = nullptr;

	mInitialized = false;
	TTF_Quit();
	SDL_Quit();
}

void lune::Engine::run()
{
	LN_LOG(Info, Engine, "Starting main loop");

	constexpr int32 frameTimeMs = 1000 / 500;

	uint64 nowTicks = SDL_GetTicks();
	uint64 nextTick = nowTicks;
	uint64 prevTick = nowTicks;

	mRunning = true;
	while (mRunning)
	{
		uint32_t nowTicks = SDL_GetTicks();
		if (nextTick > nowTicks)
		{
			SDL_Delay(nextTick - nowTicks);
		}
		nowTicks = SDL_GetTicks();

		const uint64 elapsedMs = nowTicks - prevTick;
		double deltaSeconds = elapsedMs / 1000.f;
		if (deltaSeconds > 0.1666)
			deltaSeconds = 0.1666;

		prevTick = nowTicks;
		nextTick += frameTimeMs;

		findSubsystem<EventSubsystem>()->processEvents();
		findSubsystem<TimerSubsystem>()->tick(deltaSeconds);

		for (auto& [sId, s] : mScenes)
		{
			s->update(deltaSeconds);
		}

		auto vkSubsystem = findSubsystem<VulkanSubsystem>();

		for (uint32 viewId : mViews)
		{
			if (vkSubsystem->beginNextFrame(viewId))
			{
				auto [viewId, imageIndex, commandBuffer] = vkSubsystem->getFrameInfo();

				for (auto& [sId, s] : mScenes)
				{
					s->prepareRender();
				}

				vkSubsystem->beginRenderPass();

				for (auto& [sId, s] : mScenes)
				{
					s->render();
				}

				vkSubsystem->sumbitFrame();
			}
		}
	}
}

void lune::Engine::stop()
{
	LN_LOG(Info, Engine, "Stopping main loop");
	mRunning = false;
}

uint32 lune::Engine::createWindow(std::string_view name, uint32 width, uint32 height)
{
	auto subsystem = findSubsystem<VulkanSubsystem>();
	if (subsystem)
	{
		auto newWindow = SDL_CreateWindow(name.data(), width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
		const uint32 newViewId = subsystem->createView(newWindow);
		if (newViewId != UINT32_MAX)
			mViews.emplace_back(newViewId);
		else
			SDL_DestroyWindow(newWindow);
		return newViewId;
	}
	return UINT32_MAX;
}

uint32 lune::Engine::getViewWindowId(uint32 viewId)
{
	auto subsystem = findSubsystem<VulkanSubsystem>();
	{
		auto view = subsystem->findView(viewId);
		if (view)
			return SDL_GetWindowID(view->getWindow());
	}
	return 0;
}

void lune::Engine::removeWindow(uint32 viewId)
{
	for (auto it = mViews.begin(); it != mViews.end(); it++)
	{
		if (*it == viewId)
		{
			if (auto subsystem = findSubsystem<VulkanSubsystem>(); subsystem)
				subsystem->removeView(viewId);
			mViews.erase(it);
			break;
		}
	}
}

uint64 lune::Engine::addScene(std::unique_ptr<Scene> s)
{
	static uint64 sIdCounter = 0;
	if (s) [[likely]]
		return mScenes.emplace_back(std::pair<uint64, std::unique_ptr<Scene>>{++sIdCounter, std::move(s)}).first;
	return uint64();
}

void lune::Engine::onSdlQuitEvent(const SDL_Event& event)
{
	LN_LOG(Info, Engine, "Requesting stop (SDL_EVENT_QUIT)");
	stop();
}

void lune::Engine::onSdlWindowCloseEvent(const SDL_Event& event)
{
	auto subsystem = findSubsystem<VulkanSubsystem>();
	for (auto viewId : mViews)
	{
		auto windowId = SDL_GetWindowID(subsystem->findView(viewId)->getWindow());
		if (event.window.windowID == windowId)
		{
			LN_LOG(Info, Engine, "Removing view \'{}\' (SDL_EVENT_WINDOW_CLOSE_REQUESTED)", viewId);
			removeWindow(viewId);
			break;
		}
	}
}
