#pragma once
#include "lune.hxx"

namespace lune
{
	class subsystem
	{
	public:
		subsystem() = default;
		subsystem(const subsystem&) = delete;
		subsystem(subsystem&&) = delete;
		virtual ~subsystem() = default;

		virtual bool allowInitialize() = 0;
		virtual void initialize() = 0;
		virtual void shutdown() = 0;
	};
} // namespace lune