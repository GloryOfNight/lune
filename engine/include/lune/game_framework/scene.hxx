#pragma once

#include "lune/game_framework/components/component.hxx"
#include "lune/game_framework/entities/entity.hxx"
#include "lune/game_framework/systems/system.hxx"
#include "lune/vulkan/vulkan_core.hxx"

#include <memory>
#include <unordered_map>
#include <vector>

namespace lune
{
	class Scene
	{
		struct Registry
		{
			std::unordered_map<uint64, std::shared_ptr<Entity>> entitiesIds{};
			std::unordered_map<std::type_index, std::shared_ptr<SystemBase>> systemsIds{};
		};

	public:
		Scene() = default;
		Scene(const Scene&) = delete;
		Scene(Scene&&) = default;
		virtual ~Scene() = default;

		virtual void update(double deltaTime);

		virtual void beforeRender(vk::CommandBuffer commandBuffer);
		virtual void render(vk::CommandBuffer commandBuffer);

		template <typename T, typename... Args>
		std::shared_ptr<Entity> addEntity(Args&&... args)
		{
			static uint64 eIdCounter = 0;

			auto newEntity = mEntities.emplace_back(std::make_shared<T>(std::forward<Args>(args)...));
			newEntity->assignId(++eIdCounter);

			mRegistry.entitiesIds.emplace(newEntity->getId(), newEntity);

			return newEntity;
		}

		bool detachEntity(uint64 eId)
		{
			auto findRes = mRegistry.entitiesIds.find(eId);
			if (findRes != mRegistry.entitiesIds.end())
			{
				mEntities.erase(std::find(mEntities.begin(), mEntities.end(), findRes->second));
				mRegistry.entitiesIds.erase(findRes);
				return true;
			}
			return false;
		}

		std::shared_ptr<Entity> findEntity(uint64 eId) const
		{
			auto findRes = mRegistry.entitiesIds.find(eId);
			return findRes != mRegistry.entitiesIds.end() ? findRes->second : nullptr;
		}

		template <typename T, typename... Args>
		T* registerSystem(Args&&... args)
		{
			std::type_index typeId = typeid(T);
			const auto findRes = mRegistry.systemsIds.find(typeId);
			if (findRes != mRegistry.systemsIds.end()) [[unlikely]]
				return nullptr;

			auto newSystem = mSystems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
			const auto& [it, bAdded] = mRegistry.systemsIds.emplace(std::move(typeId), newSystem);

			return dynamic_cast<T*>(newSystem.get());
		}

		template <typename T>
		T* findSystem() const
		{
			auto it = mRegistry.systemsIds.find(typeid(T));
			return it != mRegistry.systemsIds.end() ? dynamic_cast<T*>(it->second.get()) : nullptr;
		}

		const std::vector<std::shared_ptr<Entity>>& getEntities() const { return mEntities; }
		const std::vector<std::shared_ptr<SystemBase>>& getSystems() const { return mSystems; }

	private:
		std::vector<std::shared_ptr<Entity>> mEntities{};
		std::vector<std::shared_ptr<SystemBase>> mSystems{};
		Registry mRegistry{};
	};
} // namespace lune