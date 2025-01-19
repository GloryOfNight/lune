#pragma once

#include "lune/lune.hxx"

#include "engine_subsystem.hxx"

#include <functional>
#include <map>

namespace lune
{
	using TimerHandle = uint32;
	using TimerCallback = std::function<void()>;

	struct TimerManager
	{
		void tick(double deltaTime);

		TimerHandle addTimer(double seconds, TimerCallback callback, bool loop = false);

		void resetTimer(TimerHandle handle);

		void clearTimer(TimerHandle handle);

	private:
		void clearInactive();

		struct Timer
		{
			double seconds{};
			double remainingSeconds{};
			TimerCallback callback{};
			bool loop{};
			bool active{};
		};
		std::map<TimerHandle, Timer> mTimers{};
		uint32 mHandleCount{};
	};

	class TimerSubsystem final : public EngineSubsystem, public TimerManager
	{
	public:
		virtual bool allowInitialize() override { return true; };
		virtual void initialize() override {};
	};
} // namespace lune