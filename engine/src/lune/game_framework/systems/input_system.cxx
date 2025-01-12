#include "lune/game_framework/systems/input_system.hxx"

#include "lune/core/engine.hxx"
#include "lune/game_framework/components/input.hxx"
#include "lune/game_framework/scene.hxx"

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
	mBindings.push_back(eventSubsystem->addEventBindingMem(SDL_EVENT_MOUSE_BUTTON_DOWN, this, &InputSystem::onMouseButtonEvent));
	mBindings.push_back(eventSubsystem->addEventBindingMem(SDL_EVENT_MOUSE_BUTTON_UP, this, &InputSystem::onMouseButtonEvent));
	mBindings.push_back(eventSubsystem->addEventBindingMem(SDL_EVENT_MOUSE_MOTION, this, &InputSystem::onMouseMotionEvent));
	//mBindings.push_back(eventSubsystem->addEventBindingMem(SDL_EVENT_MOUSE_WHEEL, this, &InputSystem::onMouseButtonEvent));

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

	inputConfig.emplace("mouse_left_button", std::make_unique<InputActionConditionMouse>(SDL_BUTTON_LEFT, true));
}

lune::InputSystem::~InputSystem()
{
	auto eventSubsystem = Engine::get()->findSubsystem<EventSubsystem>();
	for (auto& binding : mBindings)
		eventSubsystem->removeEventBinding(binding);
	mBindings.clear();
}

void lune::InputSystem::update(Scene* scene, double deltaTime)
{
	const auto& entities = scene->getEntities();
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
					action.active = findRes->second->shouldActivate(this);
					if (wasActive != action.active)
					{
						LN_LOG(Info, Input, "Action \'{}\' state \'{}\'", action.name, action.active);
					}
				}
			}
		}
	}
}

lune::InputSystem::KeyState lune::InputSystem::getKeyState(const SDL_Keycode key) const
{
	const auto& findRes = mKeys.find(key);
	return findRes != mKeys.end() ? findRes->second : KeyState();
}

lune::InputSystem::MouseButtonState lune::InputSystem::getMouseButtonState(uint8 button) const
{
	const auto findRes = mMouseButtons.find(button);
	return findRes != mMouseButtons.end() ? findRes->second : MouseButtonState();
}

lune::InputSystem::MouseMotionState lune::InputSystem::getMouseMotionState() const
{
	return mMouseMotionState;
}

void lune::InputSystem::warpMouse(float x, float y)
{
	auto window = SDL_GetWindowFromID(mWindowId);
	if (window)
	{
		SDL_WarpMouseInWindow(window, x, y);
	}
}

void lune::InputSystem::setShowCursor(const bool state) const
{
	if (state)
		SDL_ShowCursor();
	else
		SDL_HideCursor();
}

void lune::InputSystem::onKeyEvent(const SDL_Event& event)
{
	if (event.key.windowID != mWindowId)
		return;
	mKeys.insert_or_assign(event.key.key, KeyState(event.key));
}

void lune::InputSystem::onMouseButtonEvent(const SDL_Event& event)
{
	if (event.button.windowID != mWindowId)
		return;
	mMouseButtons.insert_or_assign(event.button.button, MouseButtonState(event.button));
}

void lune::InputSystem::onMouseMotionEvent(const SDL_Event& event)
{
	if (event.motion.windowID != mWindowId)
		return;
	mMouseMotionState = MouseMotionState(event.motion);
}

void lune::InputSystem::setWindowId(uint32 windowId)
{
	mWindowId = windowId;
}