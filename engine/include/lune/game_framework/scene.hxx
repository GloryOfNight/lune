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
	public:
		scene() = default;
		scene(const scene&) = delete;
		scene(scene&&) = default;
		virtual ~scene() = default;

		template <typename T, typename... Args>
		std::shared_ptr<entity> addEntity(Args&&... args)
		{
			static uint64 eIdCounter = 0;
			const auto& [it, bAdded] = mEntities.emplace(++eIdCounter, std::make_shared<T>(std::forward<Args>(args)...));
			if (bAdded) [[likely]]
			{
				auto& [eId, e] = *it;
				e->assignId(eId);
				return e;
			}
			return nullptr;
		}

		bool detachEntity(uint64 eId)
		{
			auto findRes = mEntities.find(eId);
			if (findRes != mEntities.end())
			{
				mEntities.erase(findRes);
				return true;
			}
			return false;
		}

		std::shared_ptr<entity> findEntity(uint64 eId) const
		{
			auto findRes = mEntities.find(eId);
			return findRes != mEntities.end() ? findRes->second : nullptr;
		}

		template <typename T, typename... Args>
		system* registerSystem(Args&&... args)
		{
			std::type_index typeId = typeid(T);
			const auto findRes = mSystems.find(typeId);
			if (findRes != mSystems.end()) [[unlikely]]
				return nullptr;
			const auto& [it, bAdded] = mSystems.emplace(std::move(typeId), std::make_unique<T>(std::forward<Args>(args)...));
			return it->second.get();
		}

	private:
		std::unordered_map<uint64, std::shared_ptr<entity>> mEntities{};

		std::unordered_map<std::type_index, std::unique_ptr<system>> mSystems{};
	};
} // namespace lune