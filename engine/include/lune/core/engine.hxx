#pragma once

#include "lune/core/engine_subsystem.hxx"
#include "lune/game_framework/scene.hxx"
#include "lune/lune.hxx"

#include <memory>
#include <string>
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

		template <typename T, typename... Args>
		bool addSubsystem(Args&&... args);

		void createWindow(const std::string_view name, const uint32 width, const uint32 height, const uint32 flags = 0);

		uint64 addScene(std::unique_ptr<Scene> s);

		const std::vector<uint32>& getViewIds() const { return mViews; };

	private:
		std::vector<std::pair<uint64, std::unique_ptr<Scene>>> mScenes{};

		std::vector<std::unique_ptr<EngineSubsystem>> mSubsystems;

		std::vector<uint32> mViews{};

		std::vector<std::string> mArgs{};
	};

	template <typename T, typename... Args>
	inline bool Engine::addSubsystem(Args&&... args)
	{
		auto newSubsystem = std::make_unique<T>(std::forward<Args>(args)...);
		if (wasInitialized() && newSubsystem->allowInitialize())
		{
			const auto& emplacedSubsystem = mSubsystems.emplace_back(std::move(newSubsystem));
			emplacedSubsystem->initialize();
			return true;
		}
		else if (!wasInitialized())
		{
			mSubsystems.push_back(std::move(newSubsystem));
			return true;
		}
		return false;
	}
} // namespace lune
