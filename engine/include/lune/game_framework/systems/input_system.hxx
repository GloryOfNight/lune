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
		virtual bool isActive(class InputSystem* input) const = 0;
	};

	extern "C++" std::unordered_map<std::string, UniqueInputActionCondition>& getInputActionMapConfig();

	class InputSystem : public SystemBase
	{
	public:
		struct MouseState
		{
			SDL_MouseButtonFlags state{};
			float x{};
			float y{};
		};

		struct KeyState
		{
			KeyState() = default;
			KeyState(const SDL_Event& event)
				: timestamp{event.key.timestamp}
				, down{event.key.down}
				, repeat{event.key.repeat}
			{
			}

			uint64 timestamp;
			bool down;
			bool repeat;
		};

		InputSystem();
		InputSystem(const InputSystem&) = delete;
		InputSystem(InputSystem&&) = delete;
		virtual ~InputSystem();

		virtual void update(const std::vector<std::shared_ptr<Entity>>& entities, double deltaTime) override;

		bool isKeyPressed(const SDL_Keycode key) const;

		uint32 getMouseState(uint16* const x, uint16* const y) const;

		void warpMouse(const uint16 x, const uint16 y);

		void setShowCursor(const bool state) const;

		void setMouseRelativeMode(const bool state) const;

		bool isInMouseFocus() const;

		void setWindowId(const uint32 windowId);

	protected:
		void onKeyEvent(const SDL_Event& event);

		void onMouseEvent(const SDL_Event& event);

	private:
		std::vector<EventBindingHandle> mBindings{};

		std::map<SDL_Keycode, KeyState> mKeys{};

		MouseState mMouseState{};

		bool mHasMouseFocus{};

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

		virtual bool isActive(InputSystem* input) const override
		{
			const bool pressed = input->isKeyPressed(key);
			return pressed && down;
		}

		SDL_Keycode key{};
		bool down{};
	};

	struct InputActionConditionMouse : public InputActionConditionBase
	{
		InputActionConditionMouse() = default;
		InputActionConditionMouse(SDL_MouseButtonFlags inButton, bool inDown)
			: button{inButton}
			, down{inDown}
		{
		}

		virtual bool isActive(InputSystem* input) const override
		{
			if (input->isInMouseFocus())
				return (input->getMouseState(nullptr, nullptr) & button) == button;
			return false;
		}

		SDL_MouseButtonFlags button{};
		bool down{};
	};
} // namespace lune