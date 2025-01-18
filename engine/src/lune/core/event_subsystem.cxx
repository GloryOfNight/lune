#include "lune/core/event_subsystem.hxx"

#include "backends/imgui_impl_sdl3.h"
#include "lune/core/log.hxx"

void lune::EventSubsystem::processEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (ImGui::GetCurrentContext())
		{
			const bool bMouseEvent = event.type == SDL_EVENT_MOUSE_MOTION || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP || event.type == SDL_EVENT_MOUSE_MOTION;
			const bool bKeyBoardEvent = event.type == SDL_EVENT_KEY_UP || event.type == SDL_EVENT_KEY_DOWN;
			const auto& io = ImGui::GetIO();
			const bool bInputCaptured = ImGui_ImplSDL3_ProcessEvent(&event);
			if (bInputCaptured && io.WantCaptureMouse && bMouseEvent)
				continue;
			else if (bInputCaptured && io.WantCaptureKeyboard && bKeyBoardEvent)
				continue;
		}

		if (const auto bindIt = mEventBindings.find(event.type); bindIt != mEventBindings.end())
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

lune::EventBindingHandle lune::EventSubsystem::addEventBinding(uint32 event, EventCallbackFunc callback)
{
	auto binding = mEventBindings.try_emplace(event);
	binding.first->second.push_front(callback);

	return {event, binding.first->second.before_begin()};
}

void lune::EventSubsystem::removeEventBinding(const EventBindingHandle& handle)
{
	if (const auto bindIt = mEventBindings.find(handle.event); bindIt != mEventBindings.end())
	{
		bindIt->second.erase_after(handle.elem);
	}
}