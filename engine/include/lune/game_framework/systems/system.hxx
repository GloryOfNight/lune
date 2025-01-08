#pragma once

#include "lune/lune.hxx"

#include <memory>
#include <vector>

namespace lune
{
	class entity;

	class system
	{
	public:
		system() = default;
		system(const system&) = delete;
		system(system&&) = default;
		virtual ~system() = default;

		virtual void update(const std::vector<std::shared_ptr<entity>>& entities, double deltaTime) = 0;
	};
} // namespace lune