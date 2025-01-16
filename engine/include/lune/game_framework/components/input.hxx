#pragma once

#include "component.hxx"

#include <string>
#include <vector>

namespace lune
{
	class InputComponent : public ComponentBase
	{
	public:
		struct InputAction
		{
			std::string name{};
			bool active{};
		};

		std::vector<InputAction> actions{};
	};
} // namespace lune