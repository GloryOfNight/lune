#pragma once

#include "lune/vulkan/vulkan_core.hxx"

#include <memory>

namespace lune::vulkan
{
	using UniqueSampler = std::unique_ptr<class Sampler>;

	class Sampler
	{
	public:
		Sampler() = default;
		Sampler(Sampler&) = delete;
		Sampler(Sampler&&) = default;
		~Sampler();

		static vk::SamplerCreateInfo defaultCreateInfo();

		static UniqueSampler create(const vk::SamplerCreateInfo& createInfo);

		vk::Sampler getSampler() const { return mSampler; }

	private:
		void init(const vk::SamplerCreateInfo& createInfo);

		vk::Sampler mSampler{};
	};
} // namespace lune::vulkan