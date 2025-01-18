#include "lune/game_framework/scene.hxx"

void lune::Scene::update(double deltaTime)
{
	mRegistry.componentEntities.clear();
	for (auto& entity : mEntities)
	{
		const auto& comps = entity->getComponents();
		for (const auto& comp : comps)
		{
			auto [it, res] = mRegistry.componentEntities.try_emplace(comp.first, std::set<uint64>{});
			it->second.emplace(entity->getId());
		}
	}

	mSystemGraph.generateOrderedSystems();
	const auto systems = mSystemGraph.getOrderedSystems();
	for (auto system : systems)
	{
		system->update(this, deltaTime);
	}
}

void lune::Scene::prepareRender()
{
	const auto systems = mSystemGraph.getOrderedSystems();
	for (auto system : systems)
	{
		if (auto renderSystem = dynamic_cast<PrepareRenderSystemInterface*>(system); renderSystem)
		{
			renderSystem->prepareRender(this);
		}
	}
}

void lune::Scene::render()
{
	const auto systems = mSystemGraph.getOrderedSystems();
	for (auto system : systems)
	{
		if (auto renderSystem = dynamic_cast<RenderSystemInterface*>(system); renderSystem)
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