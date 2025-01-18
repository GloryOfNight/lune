#pragma once

#include "lune/game_framework/components/component.hxx"
#include "lune/game_framework/entities/entity.hxx"
#include "lune/game_framework/systems/system.hxx"
#include "lune/vulkan/vulkan_core.hxx"

#include "system_graph.hxx"

#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

namespace lune
{
	class Scene
	{
		struct Registry
		{
			std::unordered_map<uint64, std::set<std::unique_ptr<EntityBase>>::iterator> entitiesIds{};
			std::map<std::type_index, std::set<std::unique_ptr<SystemBase>>::iterator> systemsOrdered{};
			std::unordered_map<std::type_index, std::set<uint64>> componentEntities{};
		};

	public:
		Scene() = default;
		Scene(const Scene&) = delete;
		Scene(Scene&&) = default;
		virtual ~Scene() = default;

		virtual void update(double deltaTime);

		virtual void prepareRender();
		virtual void render();

		template <typename T, typename... Args>
		T* addEntity(Args&&... args);

		template <typename T = EntityBase>
		T* findEntity(uint64 eId) const;

		template <typename T = EntityBase>
		T* attachEntity(std::unique_ptr<T> entity);

		std::unique_ptr<EntityBase> detachEntity(uint64 eId);

		template <typename T, typename... Args>
		T* registerSystem(Args&&... args);
		template <typename T>
		T* findSystem() const;

		const std::set<std::unique_ptr<EntityBase>>& getEntities() const { return mEntities; }
		const std::set<std::unique_ptr<SystemBase>>& getSystems() const { return mSystems; }

		template <typename T>
		const std::set<uint64>& getComponentEntities();

	private:
		std::set<std::unique_ptr<EntityBase>> mEntities{};
		std::set<std::unique_ptr<SystemBase>> mSystems{};

		SystemGraph mSystemGraph{};
		Registry mRegistry{};
	};

	template <typename T, typename... Args>
	inline T* Scene::addEntity(Args&&... args)
	{
		static_assert(std::is_base_of_v<EntityBase, T>, "T must be base of Entity");
		auto newEntity = std::make_unique<T>(std::forward<Args>(args)...);
		auto eId = newEntity->getId();
		return attachEntity(std::move(newEntity));
	}

	template <typename T>
	T* Scene::findEntity(uint64 eId) const
	{
		auto findRes = mRegistry.entitiesIds.find(eId);
		return findRes != mRegistry.entitiesIds.end() ? dynamic_cast<T*>(findRes->second->get()) : nullptr;
	}

	template <typename T>
	T* Scene::attachEntity(std::unique_ptr<T> entity)
	{
		auto findRes = mRegistry.entitiesIds.find(entity->getId());
		if (findRes == mRegistry.entitiesIds.end()) [[likely]]
		{
			auto ePtr = entity.get();
			const auto& [it, res] = mEntities.emplace(std::move(entity));
			mRegistry.entitiesIds.emplace(it->get()->getId(), it);
			return dynamic_cast<T*>(ePtr);
		}
		return nullptr;
	}

	template <typename T, typename... Args>
	inline T* Scene::registerSystem(Args&&... args)
	{
		static_assert(std::is_base_of_v<SystemBase, T>, "T must be base of SystemBase");
		std::type_index typeId = typeid(T);
		const auto& [it, res] = mSystems.emplace(std::make_unique<T>(std::forward<Args>(args)...));
		if (mSystemGraph.addSystem(it)) [[likely]]
			return dynamic_cast<T*>(it->get());
		return nullptr;
	}

	template <typename T>
	inline T* Scene::findSystem() const
	{
		static_assert(std::is_base_of_v<SystemBase, T>, "T must be base of SystemBase");
		return dynamic_cast<T*>(mSystemGraph.findSystem(typeid(T)));
	}
	template <typename T>
	inline const std::set<uint64>& Scene::getComponentEntities()
	{
		static const std::set<uint64> emptySet;
		auto findRes = mRegistry.componentEntities.find(typeid(T));
		return findRes != mRegistry.componentEntities.end() ? findRes->second : emptySet;
	}
} // namespace lune