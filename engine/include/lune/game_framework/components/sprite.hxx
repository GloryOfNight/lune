#pragma once

#include "lune/core/math.hxx"

#include "component.hxx"

#include <string>

namespace lune
{
	struct SpriteComponent : public ComponentBase
	{
        std::string imageName{};
		lnm::vec3 position{};
        lnm::vec2 srcRect{}, dstRect{};
	};
} // namespace lune