#include "lune/vulkan/sampler.hxx"

lune::vulkan::Sampler::~Sampler()
{
	const auto cleanSamplerLam = [sampler = mSampler]() -> bool
	{
		getVulkanContext().device.destroySampler(sampler);
		return true;
	};
	getVulkanDeleteQueue().push(cleanSamplerLam);
}

vk::SamplerCreateInfo lune::vulkan::Sampler::defaultCreateInfo()
{
	const vk::SamplerCreateInfo samplerCreateInfo =
		vk::SamplerCreateInfo()
			.setMagFilter(vk::Filter::eLinear)
			.setMinFilter(vk::Filter::eLinear)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eRepeat)
			.setAddressModeV(vk::SamplerAddressMode::eRepeat)
			.setAddressModeW(vk::SamplerAddressMode::eRepeat)
			.setMipLodBias(0)
			.setAnisotropyEnable(VK_FALSE)
			.setMaxAnisotropy(1)
			.setCompareEnable(VK_FALSE)
			.setCompareOp(vk::CompareOp::eNever)
			.setMinLod(0)
			.setMaxLod(0)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setUnnormalizedCoordinates(VK_FALSE);
	return samplerCreateInfo;
}

lune::vulkan::UniqueSampler lune::vulkan::Sampler::create(const vk::SamplerCreateInfo& createInfo)
{
	auto newSampler = std::make_unique<Sampler>();
	newSampler->init(createInfo);
	return std::move(newSampler);
}

void lune::vulkan::Sampler::init(const vk::SamplerCreateInfo& createInfo)
{
	mSampler = getVulkanContext().device.createSampler(createInfo);
}