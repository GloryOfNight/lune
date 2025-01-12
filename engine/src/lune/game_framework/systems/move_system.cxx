#include "lune/game_framework/systems/move_system.hxx"

#include "lune/game_framework/components/input.hxx"
#include "lune/game_framework/components/move.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/entities/entity.hxx"

void lune::MoveSystem::update(const std::vector<std::shared_ptr<Entity>>& entities, double deltaTime)
{
	for (auto& entity : entities)
	{
		auto moveComp = entity->findComponent<MoveComponent>();
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
					transformComp->translate(moveComp->front * lnm::vec3(speed));
				else if (action == "move_back")
					transformComp->translate(-(moveComp->front * lnm::vec3(speed)));
				else if (action == "move_left")
					transformComp->translate(moveComp->left * lnm::vec3(speed));
				else if (action == "move_right")
					transformComp->translate(-(moveComp->left * lnm::vec3(speed)));
				else if (action == "move_up")
					transformComp->translate(moveComp->up * lnm::vec3(speed));
				else if (action == "move_down")
					transformComp->translate(-(moveComp->up * lnm::vec3(speed)));
			}
		}
	}
}