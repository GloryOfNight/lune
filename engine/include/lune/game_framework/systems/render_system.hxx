#pragma once

#include "system.hxx"

namespace lune
{
	class render_system : public system
	{
	public:
		virtual void render(const std::vector<std::shared_ptr<entity>>& entities) = 0;
	};
} // namespace lune