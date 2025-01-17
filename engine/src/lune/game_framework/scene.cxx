#include "lune/game_framework/scene.hxx"

void lune::Scene::update(double deltaTime)
{
	for (auto& system : mSystems)
	{
		system->update(this, deltaTime);
	}
}

void lune::Scene::prepareRender()
{
	for (auto& system : mSystems)
	{
		if (auto renderSystem = dynamic_cast<PrepareRenderSystemInterface*>(system.get()); renderSystem)
		{
			renderSystem->prepareRender(this);
		}
	}
}

void lune::Scene::render()
{
	for (auto& system : mSystems)
	{
		if (auto renderSystem = dynamic_cast<RenderSystemInterface*>(system.get()); renderSystem)
		{
			renderSystem->render(this);
		}
	}
}

std::unique_ptr<lune::EntityBase> lune::Scene::detachEntity(uint64 eId)
{
	auto findRes = mRegistry.entitiesIds.find(eId);
	if (findRes != mRegistry.entitiesIds.end())
	{
		auto evictedEntitiy = std::move(mEntities.extract(findRes->second).value());
		mRegistry.entitiesIds.erase(findRes);
		return std::move(evictedEntitiy);
	}
	return nullptr;
}