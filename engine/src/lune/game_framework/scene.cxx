#include "lune/game_framework/scene.hxx"

void lune::Scene::update(double deltaTime)
{
	for (auto& entity : mEntities)
	{
		entity->update(this, deltaTime);
	}

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

bool lune::Scene::attachEntity(std::shared_ptr<Entity> entity)
{
	auto findRes = mRegistry.entitiesIds.find(entity->getId());
	if (findRes == mRegistry.entitiesIds.end())
	{
		mEntities.emplace_back(entity);
		mRegistry.entitiesIds.emplace(entity->getId(), entity);
		return true;
	}
	return false;
}

std::shared_ptr<lune::Entity> lune::Scene::detachEntity(uint64 eId)
{
	auto findRes = mRegistry.entitiesIds.find(eId);
	if (findRes != mRegistry.entitiesIds.end())
	{
		auto entity = findRes->second.lock();
		mEntities.erase(std::find(mEntities.begin(), mEntities.end(), entity));
		mRegistry.entitiesIds.erase(findRes);
		return std::move(entity);
	}
	return nullptr;
}

std::shared_ptr<lune::Entity> lune::Scene::findEntity(uint64 eId) const
{
	auto findRes = mRegistry.entitiesIds.find(eId);
	return findRes != mRegistry.entitiesIds.end() ? findRes->second.lock() : nullptr;
}
