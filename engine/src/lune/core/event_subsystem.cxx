#include "lune/core/event_subsystem.hxx"

#include "lune/core/log.hxx"

void lune::EventSubsystem::processEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (const auto bindIt = eventBindings.find(event.type); bindIt != eventBindings.end())
		{
			for (auto bindFunc : bindIt->second)
			{
				try
				{
					std::invoke(bindFunc, event);
				}
				catch (std::bad_function_call badFuncCall)
				{
					LN_LOG(Error, EventSubsystem::ProcessEvents, "BadFunctuonCall: {}", badFuncCall.what());
				}
			}
		}
	}
}

lune::EventSubsystem::EventBindingHandle lune::EventSubsystem::addEventBinding(uint32 event, EventCallbackFunc callback)
{
	auto binding = eventBindings.try_emplace(event);
	binding.first->second.push_front(callback);

	return {event, binding.first->second.before_begin()};
}

void lune::EventSubsystem::removeEventBinding(const EventBindingHandle& handle)
{
	if (const auto bindIt = eventBindings.find(handle.event); bindIt != eventBindings.end())
	{
		bindIt->second.erase_after(handle.elem);
	}
}