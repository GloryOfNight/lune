#pragma once

#include "lune/core/log.hxx"
#include "lune/game_framework/components/component.hxx"
#include "lune/lune.hxx"

#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

namespace lune
{
	class Entity
	{
	public:
		Entity();
		Entity(const Entity&) = delete;
		Entity(Entity&&) = default;
		virtual ~Entity() = default;

		virtual void update(class Scene* scene, double deltaTime) = 0;

		template <typename T, typename... Args>
		T* addComponent(Args&&... args)
		{
			static_assert(std::is_base_of_v<ComponentBase, T>, "T must be base of ComponentBase");
			std::type_index typeId = typeid(T);
			const auto findRes = mComponents.find(typeId);
			if (findRes != mComponents.end()) [[unlikely]]
				return nullptr;
			const auto& [it, bAdded] = mComponents.emplace(std::move(typeId), std::make_unique<T>(std::forward<Args>(args)...));
			return dynamic_cast<T*>(it->second.get());
		}

		template <typename T>
		T* attachComponent(std::unique_ptr<T> c)
		{
			static_assert(std::is_base_of_v<ComponentBase, T>, "T must be base of ComponentBase");
			std::type_index typeId = typeid(T);
			const auto findRes = mComponents.find(typeId);
			if (c == nullptr || findRes != mComponents.end()) [[unlikely]]
				return nullptr;

			const auto& [it, bAdded] = mComponents.emplace(std::move(typeId), std::move(c));
			return dynamic_cast<T*>(it->second.get());
		}

		template <typename T>
		std::unique_ptr<ComponentBase> detachComponent()
		{
			static_assert(std::is_base_of_v<ComponentBase, T>, "T must be base of ComponentBase");
			std::type_index typeId = typeid(T);
			const auto findRes = mComponents.find(typeId);
			if (findRes == mComponents.end()) [[unlikely]]
				return nullptr;

			return std::move(mComponents.extract(findRes).mapped());
		}

		template <typename T>
		bool removeComponent()
		{
			static_assert(std::is_base_of_v<ComponentBase, T>, "T must be base of ComponentBase");
			return detachComponent<T>() != nullptr;
		}

		template <typename T>
		T* findComponent() const
		{
			static_assert(std::is_base_of_v<ComponentBase, T>, "T must be base of ComponentBase");
			auto it = mComponents.find(typeid(T));
			return it != mComponents.end() ? dynamic_cast<T*>(it->second.get()) : nullptr;
		}

		uint64 getId() const { return mId; }

	private:
		uint64 mId{};

		std::unordered_map<std::type_index, std::unique_ptr<ComponentBase>> mComponents{};
	};
} // namespace lune