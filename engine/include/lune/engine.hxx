#pragma once

#include "game_framework/scene.hxx"

#include "lune.hxx"
#include "subsystem.hxx"

#include <memory>
#include <string>
#include <vector>

namespace lune
{
	class engine final
	{
	public:
		engine() = default;
		engine(const engine&) = delete;
		engine(engine&&) = delete;
		~engine() = default;

		static engine* get();

		bool initialize(std::vector<std::string> args);

		bool wasInitialized() const;

		void shutdown();

		void run();

		template <typename T, typename... Args>
		bool addSubsystem(Args&&... args);

		void createWindow(const std::string_view name, const uint32 width, const uint32 height, const uint32 flags = 0);

		uint64 addScene(std::unique_ptr<scene> s);

	private:
		std::vector<std::pair<uint64, std::unique_ptr<scene>>> mScenes{};

		std::vector<std::unique_ptr<subsystem>> mSubsystems;

		std::vector<uint32> mViews{};

		std::vector<std::string> mArgs{};
	};

	template <typename T, typename... Args>
	inline bool engine::addSubsystem(Args&&... args)
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
