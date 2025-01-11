#include "lune/vulkan/descriptor_sets.hxx"

#include "lune/core/log.hxx"
#include "lune/vulkan/pipeline.hxx"

lune::vulkan::DescriptorSets::DescriptorSets(SharedGraphicsPipeline pipeline, uint32 maxSets)
	: DescriptorSets()
{
	mPipeline = pipeline;
	mMaxSets = maxSets;
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
	mBufferInfos.resize(mMaxSets);
	mImageInfos.resize(mMaxSets);

	createDescriptorPool();
	allocateDescriptorSets();
}

void lune::vulkan::DescriptorSets::setBufferInfo(std::string_view name, uint32 index, vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range)
{
	auto bufferInfo = vk::DescriptorBufferInfo()
						  .setBuffer(buffer)
						  .setOffset(offset)
						  .setRange(range);
	mBufferInfos[index].emplace(name, std::move(bufferInfo));
}

void lune::vulkan::DescriptorSets::setImageInfo(std::string_view name, uint32 index, vk::ImageView imageView, vk::Sampler sampler)
{
	auto imageInfo = vk::DescriptorImageInfo()
						 .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
						 .setImageView(imageView)
						 .setSampler(sampler);
	mImageInfos[index].emplace(name, std::move(imageInfo));
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

				vk::WriteDescriptorSet& write = writes.emplace_back(vk::WriteDescriptorSet())
													.setDstSet(mDescriptorSets[reflDescSet.set + descriptorSetOffset])
													.setDstBinding(reflBinding.binding)
													.setDescriptorCount(reflBinding.count)
													.setDescriptorType(static_cast<vk::DescriptorType>(reflBinding.descriptor_type));
				switch (write.descriptorType)
				{
				case vk::DescriptorType::eUniformBuffer:
					write.setPBufferInfo(&mBufferInfos[index].at(reflBinding.name));
					break;
				case vk::DescriptorType::eCombinedImageSampler:
					write.setPImageInfo(&mImageInfos[index].at(reflBinding.name));
					break;
				default:
					LN_LOG(Fatal, Vulkan::DescriptorSets, "Type {} - not supported", static_cast<int32>(write.descriptorType))
					break;
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
	const auto& Sizes = mPipeline->getDescriptorPoolSizes();

	const vk::DescriptorPoolCreateInfo createInfo = vk::DescriptorPoolCreateInfo()
														.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
														.setPoolSizes(Sizes)
														.setMaxSets(mMaxSets);

	mDescriptorPool = getVulkanContext().device.createDescriptorPool(createInfo);
}

void lune::vulkan::DescriptorSets::allocateDescriptorSets()
{
	const auto& Layouts = mPipeline->getDescriptorLayouts();
	for (uint32 i = 0; i < mMaxSets; ++i)
	{
		vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
													  .setDescriptorPool(mDescriptorPool)
													  .setSetLayouts(Layouts);

		auto newSets = getVulkanContext().device.allocateDescriptorSets(allocInfo);
		std::move(newSets.begin(), newSets.end(), std::back_inserter(mDescriptorSets));
	}
}
