#include "lune/core/timer_subsystem.hxx"

void lune::TimerManager::tick(double deltaTime)
{
	for (auto& timer : mTimers)
	{
		if (!timer.active)
			continue;

		timer.remainingSeconds -= deltaTime;
		if (timer.remainingSeconds <= 0.0)
		{
			timer.callback();

			if (timer.loop)
				timer.remainingSeconds = timer.seconds;
			else
				timer.active = false;
		}
	}
	clearInactive();
}

lune::TimerHandle lune::TimerManager::addTimer(double seconds, TimerCallback callback, bool loop)
{
	Timer newTimer{};
	newTimer.handle = ++mHandleCount;
	newTimer.seconds = seconds;
	newTimer.remainingSeconds = seconds;
	newTimer.callback = callback;
	newTimer.loop = loop;
	newTimer.active = true;

	return mTimers.emplace_back(std::move(newTimer)).handle;
}

void lune::TimerManager::resetTimer(TimerHandle handle)
{
	if (handle == TimerHandle())
		return;

	auto iter = std::find_if(mTimers.begin(), mTimers.end(), [handle](const Timer& timer)
		{ return timer.handle == handle; });

	const bool bFind = iter != mTimers.end();
	if (bFind)
	{
		iter->remainingSeconds = iter->seconds;
	}
}

void lune::TimerManager::clearTimer(TimerHandle handle)
{
	if (handle == TimerHandle())
		return;

	auto iter = std::find_if(mTimers.begin(), mTimers.end(), [handle](const Timer& timer)
		{ return timer.handle == handle; });

	const bool bFind = iter != mTimers.end();
	if (bFind)
	{
		iter->active = false;
	}
}

void lune::TimerManager::clearInactive()
{
	const auto removeIf = [](const Timer& timer)
	{ return !timer.active; };
	mTimers.erase(std::remove_if(mTimers.begin(), mTimers.end(), removeIf), mTimers.end());
}
