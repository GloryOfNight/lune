#include "descriptor_sets.hxx"

#include "pipeline.hxx"

std::unique_ptr<lune::vulkan::descriptor_sets> lune::vulkan::descriptor_sets::create()
{
	return std::make_unique<descriptor_sets>();
}

void lune::vulkan::descriptor_sets::init(std::shared_ptr<pipeline> pipeline, uint32 maxSets)
{
	mPipeline = pipeline;
	mMaxSets = maxSets;

	mBufferInfos.resize(mMaxSets);
	mImageInfos.resize(mMaxSets);

	createDescriptorPool();
	allocateDescriptorSets();
}

void lune::vulkan::descriptor_sets::destroy()
{
	if (mDescriptorPool)
		getVulkanContext().device.destroyDescriptorPool(mDescriptorPool);

	new (this) descriptor_sets();
}

void lune::vulkan::descriptor_sets::addBufferInfo(std::string_view name, uint32 index, vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range)
{
	auto bufferInfo = vk::DescriptorBufferInfo()
						  .setBuffer(buffer)
						  .setOffset(offset)
						  .setRange(range);

	mBufferInfos[index].emplace(name, std::move(bufferInfo));
}

void lune::vulkan::descriptor_sets::addImageInfo(std::string_view name, uint32 index, vk::ImageView imageView, vk::Sampler sampler)
{
	auto imageInfo = vk::DescriptorImageInfo()
						 .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
						 .setImageView(imageView)
						 .setSampler(sampler);

	mImageInfos[index].emplace(name, std::move(imageInfo));
}

void lune::vulkan::descriptor_sets::updateSets(uint32 index)
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
					break;
				}
			}
		}
	};

	createWritesLam(mPipeline->getVertShader()->getReflectModule());
	createWritesLam(mPipeline->getFragShader()->getReflectModule());

	getVulkanContext().device.updateDescriptorSets(writes, {});
}

void lune::vulkan::descriptor_sets::createDescriptorPool()
{
	const auto& Sizes = mPipeline->getDescriptorPoolSizes();

	const vk::DescriptorPoolCreateInfo createInfo = vk::DescriptorPoolCreateInfo()
														.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
														.setPoolSizes(Sizes)
														.setMaxSets(mMaxSets);

	mDescriptorPool = getVulkanContext().device.createDescriptorPool(createInfo);
}

void lune::vulkan::descriptor_sets::allocateDescriptorSets()
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
