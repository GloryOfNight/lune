#pragma once

#include "component.hxx"

#include <string>
#include <vector>

namespace lune
{
	struct InputAction
	{
		std::string name{};
		bool active{};
	};

	class InputComponent : public ComponentBase
	{
	public:
		InputComponent() = default;
		InputComponent(const InputComponent&) = delete;
		InputComponent(InputComponent&&) = default;
		~InputComponent() = default;

		std::vector<InputAction> actions{};
	};
} // namespace lune