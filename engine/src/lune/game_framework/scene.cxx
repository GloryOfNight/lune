#include "lune/game_framework/scene.hxx"

#include "lune/game_framework/systems/render_system.hxx"

void lune::Scene::update(double deltaTime)
{
	for (auto s : mSystems)
	{
		s->update(mEntities, deltaTime);
	}
}

void lune::Scene::beforeRender(vk::CommandBuffer commandBuffer)
{
	for (auto s : mSystems)
	{
		if (auto rs = dynamic_cast<RenderSystem*>(s.get()); rs)
		{
			rs->beforeRender(commandBuffer, this);
		}
	}
}

void lune::Scene::render(vk::CommandBuffer commandBuffer)
{
	for (auto s : mSystems)
	{
		if (auto rs = dynamic_cast<RenderSystem*>(s.get()); rs)
		{
			rs->render(commandBuffer, this);
		}
	}
}
