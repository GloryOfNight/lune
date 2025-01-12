#pragma once
#include "lune/lune.hxx"

#include <memory>

namespace lune
{
	using UniqueEngineSubsystem = std::unique_ptr<class EngineSubsystem>;

	class EngineSubsystem
	{
	public:
		EngineSubsystem() = default;
		EngineSubsystem(const EngineSubsystem&) = delete;
		EngineSubsystem(EngineSubsystem&&) = delete;
		virtual ~EngineSubsystem() = default;

		virtual bool allowInitialize() = 0;
		virtual void initialize() = 0;
	};
} // namespace lune