#pragma once

namespace lune
{
	struct component
	{
	public:
		component() = default;
		component(const component&) = default;
		component(component&&) = default;
		~component() = default;
	};
} // namespace lune