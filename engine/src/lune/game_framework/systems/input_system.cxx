#include "lune/game_framework/systems/input_system.hxx"

#include "lune/core/engine.hxx"
#include "lune/game_framework/components/input.hxx"

std::unordered_map<std::string, lune::UniqueInputActionCondition>& lune::getInputActionMapConfig()
{
	static std::unordered_map<std::string, UniqueInputActionCondition> config{};
	return config;
}

lune::InputSystem::InputSystem()
	: SystemBase()
{
	auto eventSubsystem = Engine::get()->findSubsystem<EventSubsystem>();
	mBindings.push_back(eventSubsystem->addEventBindingMem(SDL_EVENT_KEY_DOWN, this, &InputSystem::onKeyEvent));
	mBindings.push_back(eventSubsystem->addEventBindingMem(SDL_EVENT_KEY_UP, this, &InputSystem::onKeyEvent));
	mBindings.push_back(eventSubsystem->addEventBindingMem(SDL_EVENT_MOUSE_MOTION, this, &InputSystem::onMouseEvent));
	mBindings.push_back(eventSubsystem->addEventBindingMem(SDL_EVENT_MOUSE_BUTTON_DOWN, this, &InputSystem::onMouseEvent));
	mBindings.push_back(eventSubsystem->addEventBindingMem(SDL_EVENT_MOUSE_BUTTON_UP, this, &InputSystem::onMouseEvent));
	mBindings.push_back(eventSubsystem->addEventBindingMem(SDL_EVENT_MOUSE_WHEEL, this, &InputSystem::onMouseEvent));

	// temp; need propert input config map
	auto& inputConfig = getInputActionMapConfig();
	inputConfig.emplace("move_front", std::make_unique<InputActionConditionKey>(SDLK_W, true));
	inputConfig.emplace("move_back", std::make_unique<InputActionConditionKey>(SDLK_S, true));
	inputConfig.emplace("move_left", std::make_unique<InputActionConditionKey>(SDLK_A, true));
	inputConfig.emplace("move_right", std::make_unique<InputActionConditionKey>(SDLK_D, true));
	inputConfig.emplace("move_up", std::make_unique<InputActionConditionKey>(SDLK_SPACE, true));
	inputConfig.emplace("move_down", std::make_unique<InputActionConditionKey>(SDLK_LCTRL, true));
	inputConfig.emplace("roll_left", std::make_unique<InputActionConditionKey>(SDLK_Q, true));
	inputConfig.emplace("roll_right", std::make_unique<InputActionConditionKey>(SDLK_E, true));
	inputConfig.emplace("yaw_left", std::make_unique<InputActionConditionKey>(SDLK_LEFT, true));
	inputConfig.emplace("yaw_right", std::make_unique<InputActionConditionKey>(SDLK_RIGHT, true));
	inputConfig.emplace("pitch_up", std::make_unique<InputActionConditionKey>(SDLK_UP, true));
	inputConfig.emplace("pitch_down", std::make_unique<InputActionConditionKey>(SDLK_DOWN, true));

	inputConfig.emplace("mouse_left_button_down", std::make_unique<InputActionConditionMouse>(SDL_BUTTON_LMASK, true));
}

lune::InputSystem::~InputSystem()
{
	auto eventSubsystem = Engine::get()->findSubsystem<EventSubsystem>();
	for (auto& binding : mBindings)
		eventSubsystem->removeEventBinding(binding);
	mBindings.clear();
}

void lune::InputSystem::update(const std::vector<std::shared_ptr<Entity>>& entities, double deltaTime)
{
	const auto& inputConfig = getInputActionMapConfig();
	for (auto& entity : entities)
	{
		auto inputComp = entity->findComponent<InputComponent>();
		if (inputComp)
		{
			for (auto& action : inputComp->actions)
			{
				auto findRes = inputConfig.find(action.name);
				if (findRes != inputConfig.end())
				{
					bool wasActive = action.active;
					action.active = findRes->second->isActive(this);
					if (wasActive != action.active)
					{
						LN_LOG(Info, Input, "Action \'{}\' state \'{}\'", action.name, action.active);
					}
				}
			}
		}
	}
}

bool lune::InputSystem::isKeyPressed(const SDL_Keycode key) const
{
	const auto& findRes = mKeys.find(key);
	return findRes != mKeys.end() && findRes->second.down;
}

uint32_t lune::InputSystem::getMouseState(uint16* const x, uint16* const y) const
{
	if (x)
		*x = mMouseState.x;
	if (y)
		*y = mMouseState.y;
	return mMouseState.state;
}

void lune::InputSystem::warpMouse(const uint16 x, const uint16 y)
{
	auto window = SDL_GetWindowFromID(mWindowId);
	if (window)
	{
		SDL_WarpMouseInWindow(window, x, y);
		mMouseState.x = x;
		mMouseState.y = y;
	}
}

void lune::InputSystem::setShowCursor(const bool state) const
{
	if (state)
		SDL_ShowCursor();
	else
		SDL_HideCursor();
}

void lune::InputSystem::setMouseRelativeMode(const bool state) const
{
	setShowCursor(!state);
}

bool lune::InputSystem::isInMouseFocus() const
{
	return mHasMouseFocus;
}

void lune::InputSystem::onKeyEvent(const SDL_Event& event)
{
	SDL_Window* focusedWindow = SDL_GetMouseFocus();
	const uint32 focusedWindowId = focusedWindow ? SDL_GetWindowID(focusedWindow) : 0;
	mHasMouseFocus = mWindowId == 0 || (mWindowId == focusedWindowId);

	if (!mHasMouseFocus)
		return;

	mKeys.insert_or_assign(event.key.key, KeyState(event));
}

void lune::InputSystem::onMouseEvent(const SDL_Event& event)
{
	SDL_Window* focusedWindow = SDL_GetMouseFocus();
	const uint32 focusedWindowId = focusedWindow ? SDL_GetWindowID(focusedWindow) : 0;
	mHasMouseFocus = mWindowId == 0 || (mWindowId == focusedWindowId);

	if (!mHasMouseFocus)
		return;

	mMouseState.state = SDL_GetMouseState(&mMouseState.x, &mMouseState.y);
}

void lune::InputSystem::setWindowId(uint32 windowId)
{
	mWindowId = windowId;
}