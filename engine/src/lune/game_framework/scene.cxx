#include "lune/game_framework/scene.hxx"

void lune::Scene::update(double deltaTime)
{
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
		if (auto renderSystem = dynamic_cast<ImGuiRenderSystemInterface*>(system); renderSystem)
		{
			renderSystem->imGuiRender(this);
		}

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

		evictedEntitiy->onComponentAddedDelegate.unbind(this);
		evictedEntitiy->onComponentRemovedDelegate.unbind(this);

		const auto& comps = evictedEntitiy->getComponents();
		for (const auto& comp : comps)
			onEntityComponentRemoved(evictedEntitiy.get(), comp.second.get());

		return std::move(evictedEntitiy);
	}
	return nullptr;
}

void lune::Scene::onEntityComponentAdded(const EntityBase* entity, ComponentBase* comp)
{
	auto [it, res] = mRegistry.componentEntities.try_emplace(typeid(*comp), std::set<uint64>{});
	it->second.emplace(entity->getId());
}

void lune::Scene::onEntityComponentRemoved(const EntityBase* entity, ComponentBase* comp)
{
	const std::type_index type = typeid(*comp);
	auto findRes = mRegistry.componentEntities.find(type);
	if (findRes != mRegistry.componentEntities.end()) [[likely]]
	{
		mRegistry.componentEntities.at(type).erase(entity->getId());
	}
}
