#include "lune/game_framework/systems/camera_system.hxx"

#include "lune/game_framework/components/isometric_camera.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/entities/entity.hxx"

void lune::CameraSystem::update(const std::vector<std::shared_ptr<lune::Entity>>& entities, double deltaTime)
{
	for (const auto& e : entities)
	{
		auto isoCam = e->findComponent<IsometricCameraComponent>();
		if (isoCam)
		{
			lnm::vec3 position{};

			auto transformComp = e->findComponent<TransformComponent>();
			if (transformComp)
				position = transformComp->mPosition;

			position += isoCam->mPosition;
			const auto& direction = isoCam->mDirection;
			const auto& up = isoCam->mUp;

			mView = lnm::lookAt(position, position + direction, up);
			mProjection = isoCam->getProjection();
			mViewProjection = mView * mProjection;

			break;
		}
	}
}