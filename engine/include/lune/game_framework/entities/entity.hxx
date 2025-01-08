#pragma once

#include "lune/game_framework/components/component.hxx"
#include "lune/log.hxx"
#include "lune/lune.hxx"

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace lune
{
	class entity
	{
	public:
		entity() = default;
		entity(const entity&) = delete;
		entity(entity&&) = default;
		virtual ~entity() = default;

		template <typename T, typename... Args>
		component* addComponent(Args&&... args)
		{
			std::type_index typeId = typeid(T);
			const auto findRes = mComponents.find(typeId);
			if (findRes != mComponents.end()) [[unlikely]]
				return nullptr;
			const auto& [it, bAdded] = mComponents.emplace(std::move(typeId), std::make_unique<T>(std::forward<Args>(args)...));
			return it->second.get();
		}

		template <typename T>
		component* attachComponent(std::unique_ptr<T> c)
		{
			std::type_index typeId = typeid(T);
			const auto findRes = mComponents.find(typeId);
			if (c == nullptr || findRes != mComponents.end()) [[unlikely]]
				return nullptr;

			const auto& [it, bAdded] = mComponents.emplace(std::move(typeId), std::move(c));
			return it->second.get();
		}

		template <typename T>
		std::unique_ptr<component> detachComponent()
		{
			std::type_index typeId = typeid(T);
			const auto findRes = mComponents.find(typeId);
			if (findRes == mComponents.end()) [[unlikely]]
				return nullptr;

			return std::move(mComponents.extract(findRes).mapped());
		}

		template <typename T>
		bool removeComponent()
		{
			return detachComponent<T>() != nullptr;
		}

		template <typename T>
		T* findComponent() const
		{
			auto it = mComponents.find(typeid(T));
			return it != mComponents.end() ? dynamic_cast<T*>(it->second.get()) : nullptr;
		}

		uint64 getId() const { return mId; }

		void assignId(uint64 eId)
		{
			if (mId != 0) [[unlikely]]
				LN_LOG(Fatal, Entity, "Do not reassign eId");
			mId = eId;
		}

	private:
		uint64 mId{};

		std::unordered_map<std::type_index, std::unique_ptr<component>> mComponents{};
	};
} // namespace lune