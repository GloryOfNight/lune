#pragma once

#include "components/component.hxx"
#include "entities/entity.hxx"
#include "systems/system.hxx"

#include <memory>
#include <unordered_map>
#include <vector>

namespace lune
{
	class scene
	{
		struct registry
		{
			std::unordered_map<uint64, std::shared_ptr<entity>> entitiesIds{};
			std::unordered_map<std::type_index, std::shared_ptr<system>> systemsIds{};
		};

	public:
		scene() = default;
		scene(const scene&) = delete;
		scene(scene&&) = default;
		virtual ~scene() = default;

		virtual void update(double deltaTime);
		virtual void render();

		template <typename T, typename... Args>
		std::shared_ptr<entity> addEntity(Args&&... args)
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

		std::shared_ptr<entity> findEntity(uint64 eId) const
		{
			auto findRes = mRegistry.entitiesIds.find(eId);
			return findRes != mRegistry.entitiesIds.end() ? findRes->second : nullptr;
		}

		template <typename T, typename... Args>
		system* registerSystem(Args&&... args)
		{
			std::type_index typeId = typeid(T);
			const auto findRes = mRegistry.systemsIds.find(typeId);
			if (findRes != mRegistry.systemsIds.end()) [[unlikely]]
				return nullptr;

			auto newSystem = mSystems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
			const auto& [it, bAdded] = mRegistry.systemsIds.emplace(std::move(typeId), newSystem);

			return newSystem;
		}

	private:
		std::vector<std::shared_ptr<entity>> mEntities{};
		std::vector<std::shared_ptr<system>> mSystems{};
		registry mRegistry{};
	};
} // namespace lune