#include "game_framework/systems/camera_system.hxx"

#include "game_framework/components/isometric_camera.hxx"
#include "game_framework/components/transform.hxx"
#include "game_framework/entities/entity.hxx"

void lune::camera_system::update(const std::vector<std::shared_ptr<lune::entity>>& entities, double deltaTime)
{
	for (const auto& e : entities)
	{
		auto isoCam = e->findComponent<isometric_camera_component>();
		if (isoCam)
		{
			lnm::vec3 position{};

			auto transformComp = e->findComponent<transform_component>();
			if (transformComp)
				position = transformComp->getPosition();

			position += isoCam->getPosition();
			const auto& direction = isoCam->getDirection();
			const auto& up = isoCam->getUp();

			mView = lnm::lookAt(position, position + direction, up);
			mProjection = isoCam->getProjection();
			mViewProjection = mView * mProjection;

            break;
		}
	}
}