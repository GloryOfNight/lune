#include "lune/game_framework/systems/move_system.hxx"

#include "lune/game_framework/components/input.hxx"
#include "lune/game_framework/components/move.hxx"
#include "lune/game_framework/components/rotate.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/entities/entity.hxx"
#include "lune/game_framework/scene.hxx"
#include "lune/game_framework/systems/input_system.hxx"

#include <SDL3/SDL.h>

void lune::MoveSystem::update(Scene* scene, double deltaTime)
{
	auto inputSystem = scene->findSystem<InputSystem>();
	if (!inputSystem)
		return;

	const auto& entities = scene->getEntities();
	for (auto& entity : entities)
	{
		auto moveComp = entity->findComponent<MoveComponent>();
		auto rotComp = entity->findComponent<RotateComponent>();
		auto inputComp = entity->findComponent<InputComponent>();
		auto transformComp = entity->findComponent<TransformComponent>();
		if (moveComp && inputComp && transformComp)
		{
			const double speed = moveComp->speed * deltaTime;
			for (auto& [action, active] : inputComp->actions)
			{
				if (active && action == "mouse_left_button")
				{
					auto mouseState = inputSystem->getMouseMotionState();
					auto windowId = inputSystem->getWindowId();
					auto window = SDL_GetWindowFromID(windowId);

					int w, h;
					SDL_GetWindowSize(window, &w, &h);

					const float halfW = static_cast<float>(w) * 0.5f;
					const float halfH = static_cast<float>(h) * 0.5f;

					const float beforeZ = transformComp->mRotation.z;

					if (mouseState.x && mouseState.x != halfW)
					{
						const auto coefDist = lnm::clamp(halfW / mouseState.x - 1, -0.1f, 0.1f);
						transformComp->rotate(-lnm::radians(rotComp->speed * coefDist), transformComp->mRotation * upAxis);
					}
					if (mouseState.y && mouseState.y != halfH)
					{
						const auto coefDist = lnm::clamp(halfH / mouseState.y - 1, -0.1f, 0.1f);
						transformComp->rotate(lnm::radians(rotComp->speed * coefDist), transformComp->mRotation * rightAxis);
					}

					transformComp->mRotation.z = beforeZ;
					transformComp->mRotation = lnm::normalize(transformComp->mRotation);

					inputSystem->setShowCursor(false);
					inputSystem->warpMouse(halfW, halfH);
				}
				else if (!active && action == "mouse_left_button")
				{
					inputSystem->setShowCursor(true);
				}

				if (!active)
					continue;

				if (action == "move_front")
					transformComp->move(frontAxis * lnm::vec3(speed));
				else if (action == "move_back")
					transformComp->move(-(frontAxis * lnm::vec3(speed)));
				else if (action == "move_left")
					transformComp->move(rightAxis * lnm::vec3(speed));
				else if (action == "move_right")
					transformComp->move(-(rightAxis * lnm::vec3(speed)));
				else if (action == "move_up")
					transformComp->translate(upAxis * lnm::vec3(speed));
				else if (action == "move_down")
					transformComp->translate(-(upAxis * lnm::vec3(speed)));

				if (rotComp)
				{
					if (action == "yaw_left")
						transformComp->rotate(lnm::radians(-rotComp->speed * deltaTime), transformComp->mRotation * upAxis);
					else if (action == "yaw_right")
						transformComp->rotate(lnm::radians(rotComp->speed * deltaTime), transformComp->mRotation * upAxis);
					if (action == "pitch_up")
						transformComp->rotate(lnm::radians(rotComp->speed * deltaTime), transformComp->mRotation * rightAxis);
					else if (action == "pitch_down")
						transformComp->rotate(lnm::radians(-rotComp->speed * deltaTime), transformComp->mRotation * rightAxis);
					else if (action == "roll_left")
						transformComp->rotate(lnm::radians(rotComp->speed * deltaTime), transformComp->mRotation * frontAxis);
					else if (action == "roll_right")
						transformComp->rotate(lnm::radians(-rotComp->speed * deltaTime), transformComp->mRotation * frontAxis);
				}
			}
		}
	}
}