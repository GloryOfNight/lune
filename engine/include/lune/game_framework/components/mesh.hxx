#pragma once

#include "component.hxx"

#include <string>
#include <vector>

namespace lune
{
	struct MeshComponent : public ComponentBase
	{
		std::vector<std::string> primitiveNames{};
	};
} // namespace lune