#pragma once

#include "vulkan/vulkan.hpp"

#include "lune.hxx"
#include "subsystem.hxx"

namespace lune
{
	class vulkan_subsystem : public subsystem
	{
	public:
		vulkan_subsystem() = default;
		vulkan_subsystem(const subsystem&) = delete;
		vulkan_subsystem(subsystem&&) = delete;
		virtual ~vulkan_subsystem() = default;

		virtual bool allowInitialize() override;
		virtual void initialize() override;
		virtual void shutdown() override;

	private:
		void createInstance();

		uint32 mApiVersion{};
		vk::Instance mInstance{};
		vk::PhysicalDevice mPhysicalDevice{};
	};
} // namespace lune