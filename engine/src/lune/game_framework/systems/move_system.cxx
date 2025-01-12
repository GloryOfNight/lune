#include "lune/game_framework/systems/move_system.hxx"

#include "lune/game_framework/components/input.hxx"
#include "lune/game_framework/components/move.hxx"
#include "lune/game_framework/components/rotate.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/entities/entity.hxx"

void lune::MoveSystem::update(const std::vector<std::shared_ptr<Entity>>& entities, double deltaTime)
{
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
				if (!active)
					continue;

				if (action == "move_front")
					transformComp->move(moveComp->front * lnm::vec3(speed));
				else if (action == "move_back")
					transformComp->move(-(moveComp->front * lnm::vec3(speed)));
				else if (action == "move_left")
					transformComp->move(moveComp->left * lnm::vec3(speed));
				else if (action == "move_right")
					transformComp->move(-(moveComp->left * lnm::vec3(speed)));
				else if (action == "move_up")
					transformComp->move(moveComp->up * lnm::vec3(speed));
				else if (action == "move_down")
					transformComp->move(-(moveComp->up * lnm::vec3(speed)));

				if (rotComp)
				{
					if (action == "roll_left")
						transformComp->rotate(lnm::radians(-moveComp->rotSpeed * deltaTime), moveComp->roll);
					else if (action == "roll_right")
						transformComp->rotate(lnm::radians(moveComp->rotSpeed * deltaTime), moveComp->roll);
					if (action == "yaw_left")
						transformComp->rotate(lnm::radians(-moveComp->rotSpeed * deltaTime), moveComp->yaw);
					else if (action == "yaw_right")
						transformComp->rotate(lnm::radians(moveComp->rotSpeed * deltaTime), moveComp->yaw);
					if (action == "pitch_up")
						transformComp->rotate(lnm::radians(-moveComp->rotSpeed * deltaTime), moveComp->pitch);
					else if (action == "pitch_down")
						transformComp->rotate(lnm::radians(moveComp->rotSpeed * deltaTime), moveComp->pitch);
				}
			}
		}
	}
}