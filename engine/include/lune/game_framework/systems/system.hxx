#pragma once

#include "lune/lune.hxx"

#include <memory>
#include <vector>

namespace lune
{
	class Entity;

	class SystemBase
	{
	public:
		SystemBase() = default;
		SystemBase(const SystemBase&) = delete;
		SystemBase(SystemBase&&) = default;
		virtual ~SystemBase() = default;

		virtual void update(class Scene* scene, double deltaTime) = 0;
	};
} // namespace lune