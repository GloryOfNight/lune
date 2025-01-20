#pragma once

#include "component.hxx"

#include <string>

namespace lune
{
	struct SkyboxComponent : public ComponentBase
	{
        std::string imageName{};
	};
} // namespace lune