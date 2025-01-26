#include "lune/vulkan/descriptor_sets.hxx"

#include "lune/core/log.hxx"
#include "lune/vulkan/pipeline.hxx"

lune::vulkan::DescriptorSets::DescriptorSets(SharedGraphicsPipeline pipeline, uint32 maxSets)
	: DescriptorSets()
{
	mPipeline = pipeline;
	mMaxAllocations = maxSets;
}

lune::vulkan::DescriptorSets::~DescriptorSets()
{
	const auto cleanDescPoolLam = [descPool = mDescriptorPool]() -> bool
	{
		getVulkanContext().device.destroyDescriptorPool(descPool);
		return true;
	};
	getVulkanDeleteQueue().push(cleanDescPoolLam);
}

lune::vulkan::UniqueDescriptorSets lune::vulkan::DescriptorSets::create(SharedGraphicsPipeline pipeline, uint32 maxSets)
{
	auto newDescSets = std::make_unique<DescriptorSets>(pipeline, maxSets);
	newDescSets->init();
	return std::move(newDescSets);
}

void lune::vulkan::DescriptorSets::init()
{
	mBufferInfos.resize(mMaxAllocations);
	mImageInfos.resize(mMaxAllocations);

	createDescriptorPool();
	allocateDescriptorSets();
}

void lune::vulkan::DescriptorSets::setBufferInfo(std::string_view name, uint32 allocId, vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range)
{
	auto bufferInfo = vk::DescriptorBufferInfo()
						  .setBuffer(buffer)
						  .setOffset(offset)
						  .setRange(range);
	mBufferInfos[allocId].insert_or_assign(std::string(name), std::move(bufferInfo));
}

void lune::vulkan::DescriptorSets::setImageInfo(std::string_view name, uint32 allocId, vk::ImageView imageView, vk::Sampler sampler, uint32 dstArrayElem)
{
	auto imageInfo = vk::DescriptorImageInfo()
						 .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
						 .setImageView(imageView)
						 .setSampler(sampler);
	auto [it, res] = mImageInfos[allocId].try_emplace(std::string(name));
	it->second.insert_or_assign(dstArrayElem, std::move(imageInfo));
}

void lune::vulkan::DescriptorSets::updateSets(uint32 index)
{
	const uint32 descriptorSetOffset = mPipeline->getDescriptorLayouts().size() * index;

	std::vector<vk::WriteDescriptorSet> writes;

	const auto createWritesLam = [&](const SpvReflectShaderModule refl)
	{
		for (uint32 i = 0; i < refl.descriptor_set_count; i++)
		{
			const auto& reflDescSet = refl.descriptor_sets[i];
			for (uint32 k = 0; k < reflDescSet.binding_count; k++)
			{
				const auto& reflBinding = refl.descriptor_bindings[k];

				vk::WriteDescriptorSet write = vk::WriteDescriptorSet()
												   .setDstSet(mDescriptorSets[reflDescSet.set + descriptorSetOffset])
												   .setDstBinding(reflBinding.binding)
												   .setDstArrayElement(0)
												   .setDescriptorCount(reflBinding.count)
												   .setDescriptorType(static_cast<vk::DescriptorType>(reflBinding.descriptor_type));

				if (write.descriptorType == vk::DescriptorType::eUniformBuffer)
				{
					write.setBufferInfo(mBufferInfos[index].at(reflBinding.name));
					writes.emplace_back(write);
				}
				else if (write.descriptorType == vk::DescriptorType::eCombinedImageSampler)
				{
					const auto& images = mImageInfos[index].at(reflBinding.name);
					for (const auto& [elem, image] : images)
					{
						write.setImageInfo(image);
						write.setDstArrayElement(elem);
						writes.emplace_back(write);
					}
				}
				else
				{
					LN_LOG(Fatal, Vulkan::DescriptorSets, "Type {} - not supported", static_cast<int32>(write.descriptorType))
				}
			}
		}
	};

	createWritesLam(mPipeline->getVertShader()->getReflectModule());
	createWritesLam(mPipeline->getFragShader()->getReflectModule());

	getVulkanContext().device.updateDescriptorSets(writes, {});
}

void lune::vulkan::DescriptorSets::cmdBind(vk::CommandBuffer commandBuffer, uint32 offsetSets)
{
	uint32 count = mPipeline->getDescriptorLayouts().size();
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipeline->getPipelineLayout(), 0, count, mDescriptorSets.data() + offsetSets, 0, nullptr);
}

void lune::vulkan::DescriptorSets::createDescriptorPool()
{
	const auto& poolSizes = mPipeline->getDescriptorPoolSizes();
	const uint32 maxSets = mPipeline->getDescriptorLayouts().size() * mMaxAllocations;

	const vk::DescriptorPoolCreateInfo createInfo = vk::DescriptorPoolCreateInfo()
														.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
														.setPoolSizes(poolSizes)
														.setMaxSets(maxSets);

	mDescriptorPool = getVulkanContext().device.createDescriptorPool(createInfo);
}

void lune::vulkan::DescriptorSets::allocateDescriptorSets()
{
	const auto& Layouts = mPipeline->getDescriptorLayouts();
	for (uint32 i = 0; i < mMaxAllocations; ++i)
	{
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
													  .setDescriptorPool(mDescriptorPool)
													  .setSetLayouts(Layouts);

		auto newSets = getVulkanContext().device.allocateDescriptorSets(allocInfo);
		std::move(newSets.begin(), newSets.end(), std::back_inserter(mDescriptorSets));
	}
}
