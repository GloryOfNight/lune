#pragma once

#include "lune/core/event_subsystem.hxx"
#include "lune/lune.hxx"

#include "system.hxx"

#include <SDL3/SDL_events.h>
#include <map>
#include <memory>
#include <unordered_map>

namespace lune
{
	using UniqueInputActionCondition = std::unique_ptr<struct InputActionConditionBase>;
	struct InputActionConditionBase
	{
		virtual ~InputActionConditionBase() = default;
		virtual bool shouldActivate(class InputSystem* input) const = 0;
	};

	extern "C++" std::unordered_map<std::string, UniqueInputActionCondition>& getInputActionMapConfig();

	class InputSystem : public SystemBase
	{
	public:
		struct KeyState
		{
			KeyState() = default;
			KeyState(const SDL_KeyboardEvent& event)
				: timestamp{event.timestamp}
				, down{event.down}
				, repeat{event.repeat}
			{
			}

			uint64 timestamp;
			bool down;
			bool repeat;
		};

		struct MouseButtonState
		{
			MouseButtonState() = default;
			MouseButtonState(const SDL_MouseButtonEvent& event)
				: timestamp{event.timestamp}
				, clicks{event.clicks}
				, down{event.down}
				, x{event.x}
				, y{event.y}
			{
			}

			uint64 timestamp;
			uint8 clicks;
			bool down;
			float x, y;
		};

		struct MouseMotionState
		{
			MouseMotionState() = default;
			MouseMotionState(const SDL_MouseMotionEvent& event)
				: x{event.x}
				, y{event.y}
				, xrel{event.xrel}
				, yrel{event.yrel}
			{
			}

			float x, y, xrel, yrel;
		};

		InputSystem();
		InputSystem(const InputSystem&) = delete;
		InputSystem(InputSystem&&) = delete;
		virtual ~InputSystem();

		virtual void update(class Scene* scene, double deltaTime) override;

		KeyState getKeyState(const SDL_Keycode key) const;
		MouseButtonState getMouseButtonState(uint8 button) const;
		MouseMotionState getMouseMotionState() const;

		void warpMouse(float x, float y);

		void setShowCursor(const bool state) const;

		void setWindowId(const uint32 windowId);
		uint32 getWindowId() const { return mWindowId; }

	protected:
		void onKeyEvent(const SDL_Event& event);
		void onMouseButtonEvent(const SDL_Event& event);
		void onMouseMotionEvent(const SDL_Event& event);

	private:
		std::vector<EventBindingHandle> mBindings{};

		std::map<SDL_Keycode, KeyState> mKeys{};

		std::map<uint8, MouseButtonState> mMouseButtons{};

		MouseMotionState mMouseMotionState{};

		uint32 mWindowId{};
	};

	struct InputActionConditionKey : public InputActionConditionBase
	{
		InputActionConditionKey() = default;
		InputActionConditionKey(SDL_Keycode inKey, bool inDown)
			: key{inKey}
			, down{inDown}
		{
		}

		virtual bool shouldActivate(InputSystem* input) const override
		{
			const auto keyState = input->getKeyState(key);
			return keyState.down && down;
		}

		SDL_Keycode key{};
		bool down{};
	};

	struct InputActionConditionMouse : public InputActionConditionBase
	{
		InputActionConditionMouse() = default;
		InputActionConditionMouse(uint8 inButton, bool inDown)
			: button{inButton}
			, down{inDown}
		{
		}

		virtual bool shouldActivate(InputSystem* input) const override
		{
			const auto mouseState = input->getMouseButtonState(button);
			return mouseState.down == down;
		}

		uint8 button{};
		bool down{};
	};
} // namespace lune