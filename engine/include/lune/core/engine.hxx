#pragma once

#include "SDL3/SDL_events.h"
#include "lune/core/engine_subsystem.hxx"
#include "lune/core/log.hxx"
#include "lune/game_framework/scene.hxx"
#include "lune/lune.hxx"

#include <memory>
#include <string>
#include <typeindex>
#include <vector>

namespace lune
{
	class Engine final
	{
	public:
		Engine() = default;
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		~Engine() = default;

		static Engine* get();

		bool initialize(std::vector<std::string> args);

		bool wasInitialized() const;

		void shutdown();

		void run();
		void stop();

		uint32 createWindow(std::string_view name, uint32 width, uint32 height);
		uint32 getViewWindowId(uint32 viewId);
		void removeWindow(uint32 viewId);

		Scene* addScene(std::unique_ptr<Scene> s);

		const std::vector<uint32>& getViewIds() const { return mViews; };

		template <typename T, typename... Args>
		T* addSubsystem(Args&&... args);

		template <typename T>
		T* findSubsystem();

	private:
		void onSdlQuitEvent(const SDL_Event& event);
		void onSdlWindowCloseEvent(const SDL_Event& event);
		void onSdlWindowPixelSizeChanged(const SDL_Event& event);

		std::vector<std::pair<uint64, std::unique_ptr<Scene>>> mScenes{};

		std::unordered_map<std::type_index, UniqueEngineSubsystem> mSubsystems;

		std::vector<uint32> mViews{};

		std::vector<std::string> mArgs{};

		bool mInitialized{false};
		bool mRunning{false};
	};

	template <typename T, typename... Args>
	inline T* Engine::addSubsystem(Args&&... args)
	{
		std::type_index typeIndex = typeid(T);
		if (findSubsystem<T>()) [[unlikely]]
		{
			LN_LOG(Fatal, Engine, "Subsystem \'{}\' already added", typeIndex.name())
		}

		auto newSubsystem = std::make_unique<T>(std::forward<Args>(args)...);
		if (newSubsystem->allowInitialize())
		{
			const auto& [it, result] = mSubsystems.emplace(typeIndex, std::move(newSubsystem));
			it->second->initialize();
			return dynamic_cast<T*>(it->second.get());
		}
		else
		{
			LN_LOG(Fatal, Engine, "Subsystem \'{}\' cannot be initialized ", typeIndex.name())
		}
		return nullptr;
	}

	template <typename T>
	inline T* Engine::findSubsystem()
	{
		auto it = mSubsystems.find(typeid(T));
		return it != mSubsystems.end() ? dynamic_cast<T*>(it->second.get()) : nullptr;
	}
} // namespace lune
