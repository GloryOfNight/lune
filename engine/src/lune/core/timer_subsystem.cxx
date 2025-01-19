#include "lune/core/timer_subsystem.hxx"

void lune::TimerManager::tick(double deltaTime)
{
	for (auto& [handle, timer] : mTimers)
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
	newTimer.seconds = seconds;
	newTimer.remainingSeconds = seconds;
	newTimer.callback = callback;
	newTimer.loop = loop;
	newTimer.active = true;
	mTimers.emplace(++mHandleCount, std::move(newTimer));
	return mHandleCount;
}

void lune::TimerManager::resetTimer(TimerHandle handle)
{
	auto findRes = mTimers.find(handle);
	if (findRes != mTimers.end())
		findRes->second.remainingSeconds = findRes->second.seconds;
}

void lune::TimerManager::clearTimer(TimerHandle handle)
{
	auto findRes = mTimers.find(handle);
	if (findRes != mTimers.end())
		findRes->second.active = false;
}

void lune::TimerManager::clearInactive()
{
	for (auto it = mTimers.begin(); it != mTimers.end();)
	{
		if (!it->second.active)
			it = mTimers.erase(it);
		else
			++it;
	}
}
