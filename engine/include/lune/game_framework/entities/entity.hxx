#pragma once

#include "lune/core/delegate.hxx"
#include "lune/game_framework/components/component.hxx"
#include "lune/lune.hxx"

#include <map>
#include <memory>
#include <type_traits>
#include <typeindex>

namespace lune
{
	class EntityBase
	{
	public:
		using ComponentDelegate = DelegateOwned<EntityBase, const EntityBase*, ComponentBase*>;

		EntityBase();
		EntityBase(const EntityBase&) = delete;
		EntityBase(EntityBase&&) = default;
		virtual ~EntityBase() = default;

		ComponentDelegate onComponentAddedDelegate;
		ComponentDelegate onComponentRemovedDelegate;

		template <typename T, typename... Args>
		T* addComponent(Args&&... args)
		{
			static_assert(std::is_base_of_v<ComponentBase, T>, "T must be base of ComponentBase");
			std::type_index typeId = typeid(T);
			const auto findRes = mComponents.find(typeId);
			if (findRes != mComponents.end()) [[unlikely]]
				return nullptr;
			const auto& [it, bAdded] = mComponents.emplace(std::move(typeId), std::make_unique<T>(std::forward<Args>(args)...));
			onComponentAddedDelegate.execute(this, it->second.get());
			return dynamic_cast<T*>(it->second.get());
		}

		template <typename T>
		bool removeComponent()
		{
			static_assert(std::is_base_of_v<ComponentBase, T>, "T must be base of ComponentBase");
			std::type_index typeId = typeid(T);
			const auto findRes = mComponents.find(typeId);
			if (findRes == mComponents.end()) [[unlikely]]
				return false;
			auto comp = std::move(mComponents.extract(findRes).mapped());
			onComponentRemovedDelegate.execute(this, comp.get());
			return true;
		}

		template <typename T>
		T* findComponent() const
		{
			static_assert(std::is_base_of_v<ComponentBase, T>, "T must be base of ComponentBase");
			auto it = mComponents.find(typeid(T));
			return it != mComponents.end() ? dynamic_cast<T*>(it->second.get()) : nullptr;
		}

		uint64 getId() const { return mId; }

		const std::map<std::type_index, std::unique_ptr<ComponentBase>>& getComponents() const { return mComponents; }

	private:
		uint64 mId{};

		std::map<std::type_index, std::unique_ptr<ComponentBase>> mComponents{};
	};
} // namespace lune