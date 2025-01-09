#pragma once
#include "lune/lune.hxx"

namespace lune
{
	class EngineSubsystem
	{
	public:
		EngineSubsystem() = default;
		EngineSubsystem(const EngineSubsystem&) = delete;
		EngineSubsystem(EngineSubsystem&&) = delete;
		virtual ~EngineSubsystem() = default;

		virtual bool allowInitialize() = 0;
		virtual void initialize() = 0;
		virtual void shutdown() = 0;
	};
} // namespace lune