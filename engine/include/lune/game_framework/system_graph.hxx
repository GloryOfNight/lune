#pragma once

#include "lune/core/log.hxx"
#include "lune/game_framework/systems/system.hxx"
#include "lune/lune.hxx"

#include <memory>
#include <set>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace lune
{
	struct SystemGraph
	{
		bool addSystem(std::set<std::unique_ptr<SystemBase>>::iterator system)
		{
			const auto& systemRef = *system->get();
			const std::type_index type = std::type_index(typeid(systemRef));
			if (mSystemsTypes.find(type) != mSystemsTypes.end()) [[unlikely]]
			{
				LN_LOG(Fatal, SystemGraph, "Attempt to add same system twice!");
				return false;
			}

			mSystemsTypes.emplace(type, system);
			mSystemDependecies.emplace(type, std::unordered_set<std::type_index>{});

			const auto& systemDeps = system->get()->getDependecies();
			for (const auto& dep : systemDeps)
			{
				mSystemDependecies.at(type).insert(dep);
			}
			return true;
		}

		SystemBase* findSystem(std::type_index type) const
		{
			const auto findRes = mSystemsTypes.find(type);
			return findRes != mSystemsTypes.end() ? findRes->second->get() : nullptr;
		}

		std::vector<SystemBase*> getOrderedSystems() const { return mOrderedSystems; }

		void generateOrderedSystems()
		{
			mOrderedSystems.clear();
			std::set<std::type_index> satisfiedTypes{};
			std::set<std::type_index> ignoredTypes{};
			mOrderedSystems.reserve(mSystemsTypes.size());

			while (mOrderedSystems.size() != (mSystemsTypes.size() - ignoredTypes.size()))
			{
				for (const auto& [type, deps] : mSystemDependecies)
				{
					if (satisfiedTypes.contains(type))
						continue;
					if (ignoredTypes.contains(type))
						continue;

					bool bDepsSatisfied = true;
					for (const auto dep : deps)
					{
						if (mSystemsTypes.find(dep) == mSystemsTypes.end()) // dependency not present at current graph
						{
							ignoredTypes.emplace(dep);
							bDepsSatisfied = false;
							break;
						}
						else if (mSystemDependecies.at(dep).contains(type)) // dependency has a depency for our type
						{
							ignoredTypes.emplace(dep);
							bDepsSatisfied = false;
                            LN_LOG(Fatal, SystemGraph, "Cycle dependecies!");
							break;
						}
						else if (!satisfiedTypes.contains(dep))
						{
							bDepsSatisfied = false;
							break;
						}
					}

					if (bDepsSatisfied)
					{
						mOrderedSystems.emplace_back(mSystemsTypes.find(type)->second->get());
						satisfiedTypes.emplace(type);
					}
				}
			}
		}

	private:
		std::unordered_map<std::type_index, std::set<std::unique_ptr<SystemBase>>::iterator> mSystemsTypes{};
		std::unordered_map<std::type_index, std::unordered_set<std::type_index>> mSystemDependecies{};
		std::vector<SystemBase*> mOrderedSystems{};
	};
} // namespace lune