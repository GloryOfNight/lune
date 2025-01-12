#pragma once

#include "lune/lune.hxx"

#include "engine_subsystem.hxx"

#include <SDL3/SDL_events.h>
#include <cstdint>
#include <forward_list>
#include <functional>
#include <map>

namespace lune
{
	using EventCallbackFunc = std::function<void(const SDL_Event&)>;

	struct EventBindingHandle
	{
		const uint32_t event{};
		const std::forward_list<EventCallbackFunc>::iterator elem{};
	};

	class EventSubsystem final : public EngineSubsystem
	{
	public:
		EventSubsystem() = default;
		EventSubsystem(const EventSubsystem&) = delete;
		EventSubsystem(EventSubsystem&&) = delete;
		virtual ~EventSubsystem() = default;

		virtual bool allowInitialize() override { return true; };
		virtual void initialize() override {};

		void processEvents();

		template <typename T, typename F>
		EventBindingHandle addEventBindingMem(const uint32 event, T* obj, F func);

		EventBindingHandle addEventBinding(uint32 event, EventCallbackFunc callback);

		void removeEventBinding(const EventBindingHandle& handle);

	private:
		std::map<uint32, std::forward_list<EventCallbackFunc>> mEventBindings{};
	};

	template <typename T, typename F>
	inline EventBindingHandle EventSubsystem::addEventBindingMem(const uint32 event, T* obj, F func)
	{
		return addEventBinding(event, std::bind(func, obj, std::placeholders::_1));
	}
} // namespace lune