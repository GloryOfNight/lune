#pragma once

namespace lune
{
	struct ComponentBase
	{
		ComponentBase() = default;
		ComponentBase(const ComponentBase&) = default;
		ComponentBase(ComponentBase&&) = default;
		virtual ~ComponentBase() = default;
	};
} // namespace lune