#pragma once

#include "lune/game_framework/components/component.hxx"
#include "lune/game_framework/entities/entity.hxx"
#include "lune/game_framework/systems/system.hxx"
#include "lune/vulkan/vulkan_core.hxx"

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
			std::unordered_map<uint64, std::weak_ptr<Entity>> entitiesIds{};
			std::unordered_map<std::type_index, SystemBase*> systemsIds{};
			std::unordered_map<std::type_index, std::set<std::weak_ptr<Entity>>> componentEntities{}; // todo;
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
		std::shared_ptr<Entity> addEntity(Args&&... args);
		bool attachEntity(std::shared_ptr<Entity> entity);
		std::shared_ptr<Entity> detachEntity(uint64 eId);
		std::shared_ptr<Entity> findEntity(uint64 eId) const;

		template <typename T, typename... Args>
		T* registerSystem(Args&&... args);
		template <typename T>
		T* findSystem() const;

		const std::vector<std::shared_ptr<Entity>>& getEntities() const { return mEntities; }
		const std::vector<std::unique_ptr<SystemBase>>& getSystems() const { return mSystems; }

	private:
		std::vector<std::shared_ptr<Entity>> mEntities{};
		std::vector<std::unique_ptr<SystemBase>> mSystems{};
		Registry mRegistry{};
	};

	template <typename T, typename... Args>
	inline std::shared_ptr<Entity> Scene::addEntity(Args&&... args)
	{
		static_assert(std::is_base_of_v<Entity, T>, "T must be base of Entity");
		auto newEntity = std::make_shared<T>(std::forward<Args>(args)...);
		if (attachEntity(newEntity)) [[likely]]
			return newEntity;
		LN_LOG(Fatal, Engine::Scene, "Failed to attach newly created entity!");
		return nullptr;
	}

	template <typename T, typename... Args>
	inline T* Scene::registerSystem(Args&&... args)
	{
		static_assert(std::is_base_of_v<SystemBase, T>, "T must be base of SystemBase");
		std::type_index typeId = typeid(T);
		const auto findRes = mRegistry.systemsIds.find(typeId);
		if (findRes != mRegistry.systemsIds.end()) [[unlikely]]
			return nullptr;

		auto& newSystem = mSystems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
		const auto& [it, bAdded] = mRegistry.systemsIds.emplace(std::move(typeId), newSystem.get());

		return dynamic_cast<T*>(newSystem.get());
	}

	template <typename T>
	inline T* Scene::findSystem() const
	{
		static_assert(std::is_base_of_v<SystemBase, T>, "T must be base of SystemBase");
		auto it = mRegistry.systemsIds.find(typeid(T));
		return it != mRegistry.systemsIds.end() ? dynamic_cast<T*>(it->second) : nullptr;
	}
} // namespace lune