#pragma once

#include "lune/lune.hxx"

#include "component.hxx"

#include <set>

namespace lune
{
	struct ParentChildComponent : public ComponentBase
	{
		uint64 mParentId{};
		std::set<uint64> mChildren{};
	};
} // namespace lune