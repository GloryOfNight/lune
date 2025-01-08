#pragma once

namespace lune
{
	struct component
	{
		component() = default;
		component(const component&) = default;
		component(component&&) = default;
		virtual ~component() = default;
	};
} // namespace lune