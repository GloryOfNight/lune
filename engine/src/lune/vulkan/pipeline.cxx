#include "lune/vulkan/pipeline.hxx"

#include "lune/core/log.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

#include <utility>

lune::vulkan::GraphicsPipeline::~GraphicsPipeline()
{
	const auto cleanPipelineLam = [pipeline = mPipeline, pipelineLayout = mPipelineLayout]() -> bool
	{
		getVulkanContext().device.destroyPipeline(pipeline);
		getVulkanContext().device.destroyPipelineLayout(pipelineLayout);
		return true;
	};
	const auto cleanDescriptorLayouts = [layouts = mDescriptorSetLayouts]() -> bool
	{
		for (const auto& layout : layouts)
		{
			getVulkanContext().device.destroyDescriptorSetLayout(layout);
		}
		return true;
	};

	getVulkanDeleteQueue().push(cleanPipelineLam);
	getVulkanDeleteQueue().push(cleanDescriptorLayouts);
}

std::pair<std::vector<vk::VertexInputAttributeDescription>, std::vector<vk::VertexInputBindingDescription>> reflVertexInput(const SpvReflectShaderModule& reflModule)
{
	std::vector<vk::VertexInputAttributeDescription> vertAttrs = std::vector<vk::VertexInputAttributeDescription>(reflModule.input_variable_count, vk::VertexInputAttributeDescription());
	std::vector<size_t> sizes = std::vector<size_t>(reflModule.input_variable_count, size_t());

	std::vector<vk::VertexInputBindingDescription> vertBindings = std::vector<vk::VertexInputBindingDescription>(1, vk::VertexInputBindingDescription());

	for (int i = 0; i < reflModule.input_variable_count; ++i)
	{
		const auto& inputVar = reflModule.input_variables[i];
		vertAttrs[inputVar->location] = vk::VertexInputAttributeDescription()
											.setBinding(0)
											.setLocation(inputVar->location)
											.setFormat(static_cast<vk::Format>(inputVar->format));
		sizes[inputVar->location] = (inputVar->numeric.scalar.width / 8) * inputVar->numeric.vector.component_count;
	}

	vertBindings[0] = vk::VertexInputBindingDescription()
						  .setBinding(0)
						  .setInputRate(vk::VertexInputRate::eVertex);

	for (size_t i = 0; i < reflModule.input_variable_count; ++i)
	{
		if (i > 0)
			vertAttrs[i].offset = vertBindings[0].stride;
		vertBindings[0].stride += sizes[i];
	}

	return {vertAttrs, vertBindings};
}

vk::PipelineShaderStageCreateInfo reflShaderStage(const std::shared_ptr<lune::vulkan::Shader>& shader)
{
	return vk::PipelineShaderStageCreateInfo()
		.setModule(shader->getShaderModule())
		.setStage(static_cast<vk::ShaderStageFlagBits>(shader->getReflectModule().shader_stage))
		.setPName(shader->getReflectModule().entry_point_name);
}

std::vector<std::vector<vk::DescriptorSetLayoutBinding>> relfDescriptorSetBindings(const SpvReflectShaderModule& reflModule)
{
	std::vector<std::vector<vk::DescriptorSetLayoutBinding>> result{reflModule.descriptor_set_count, std::vector<vk::DescriptorSetLayoutBinding>()};

	for (uint32 i = 0; i < reflModule.descriptor_set_count; i++)
	{
		const auto& reflSet = reflModule.descriptor_sets[i];
		auto& set = result[i];

		set = std::vector<vk::DescriptorSetLayoutBinding>(reflSet.binding_count, vk::DescriptorSetLayoutBinding());
		for (uint32 k = 0; k < reflSet.binding_count; k++)
		{
			const auto& reflBinding = reflModule.descriptor_bindings[k];
			auto& binding = set[k];

			binding = vk::DescriptorSetLayoutBinding()
						  .setBinding(reflBinding.binding)
						  .setDescriptorType(static_cast<vk::DescriptorType>(reflBinding.descriptor_type))
						  .setDescriptorCount(reflBinding.count)
						  .setStageFlags(static_cast<vk::ShaderStageFlagBits>(reflModule.shader_stage));
		}
	}

	return result;
}

std::vector<vk::PushConstantRange> reflPushConstantRanges(const SpvReflectShaderModule& reflModule)
{
	std::vector<vk::PushConstantRange> result(reflModule.push_constant_block_count, vk::PushConstantRange());

	for (uint8_t i = 0; i < reflModule.push_constant_block_count; i++)
	{
		const auto& reflPushConstantRange = reflModule.push_constant_blocks[i];
		auto& pushConstantRange = result[i];

		pushConstantRange = vk::PushConstantRange()
								.setStageFlags(static_cast<vk::ShaderStageFlagBits>(reflModule.shader_stage))
								.setOffset(reflPushConstantRange.offset)
								.setSize(reflPushConstantRange.size);
	}

	return result;
}

lune::vulkan::SharedGraphicsPipeline lune::vulkan::GraphicsPipeline::create(std::shared_ptr<Shader> vertShader, std::shared_ptr<Shader> fragShader)
{
	if (!vertShader || vertShader->getReflectModule().shader_stage != SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
	{
		LN_LOG(Fatal, Vulkan::Pipeline, "Failed to initialize pipeline: vertex shader invalid");
		return nullptr;
	}

	if (!fragShader || fragShader->getReflectModule().shader_stage != SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT)
	{
		LN_LOG(Fatal, Vulkan::Pipeline, "Failed to initialize pipeline: fragment shader invalid");
		return nullptr;
	}

	auto newPipeline = std::make_shared<GraphicsPipeline>();
	newPipeline->init(vertShader, fragShader);
	return std::move(newPipeline);
}

void lune::vulkan::GraphicsPipeline::init(std::shared_ptr<Shader> vertShader, std::shared_ptr<Shader> fragShader)
{
	mVertShader = vertShader;
	mFragShader = fragShader;

	createDescriptorLayoutsAndPoolSizes();
	createPipelineLayout();
	createPipeline();
}

void lune::vulkan::GraphicsPipeline::cmdBind(vk::CommandBuffer commandBuffer)
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);
}

void lune::vulkan::GraphicsPipeline::createDescriptorLayoutsAndPoolSizes()
{
	std::vector<std::vector<vk::DescriptorSetLayoutBinding>> totalBindings{};
	{
		const auto shaderBinds = relfDescriptorSetBindings(mVertShader->getReflectModule());
		std::move(shaderBinds.begin(), shaderBinds.end(), std::back_inserter(totalBindings));
	}
	{
		const auto shaderBinds = relfDescriptorSetBindings(mFragShader->getReflectModule());
		std::move(shaderBinds.begin(), shaderBinds.end(), std::back_inserter(totalBindings));
	}

	std::map<vk::DescriptorType, uint32> descriptorTypesCount{};

	for (const auto& bindings : totalBindings)
	{
		const auto layoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
										  .setBindings(bindings);

		mDescriptorSetLayouts.push_back(getVulkanContext().device.createDescriptorSetLayout(layoutCreateInfo));

		for (const auto& binding : bindings)
		{
			const auto [it, result] = descriptorTypesCount.try_emplace(binding.descriptorType, 0);
			auto& [type, count] = *it;
			count += binding.descriptorCount;
		}
	}

	for (auto [type, count] : descriptorTypesCount)
	{
		mPoolSizes.push_back(vk::DescriptorPoolSize()
				.setType(type)
				.setDescriptorCount(count)); // might be missing * 2!
	}
}

void lune::vulkan::GraphicsPipeline::createPipelineLayout()
{
	std::vector<vk::PushConstantRange> pushConstantRanges{};
	{
		const auto shaderRanges = reflPushConstantRanges(mVertShader->getReflectModule());
		std::move(shaderRanges.begin(), shaderRanges.end(), std::back_inserter(pushConstantRanges));
	}
	{
		const auto shaderRanges = reflPushConstantRanges(mFragShader->getReflectModule());
		std::move(shaderRanges.begin(), shaderRanges.end(), std::back_inserter(pushConstantRanges));
	}

	const vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo =
		vk::PipelineLayoutCreateInfo()
			.setSetLayouts(mDescriptorSetLayouts)
			.setPushConstantRanges(pushConstantRanges);

	mPipelineLayout = getVulkanContext().device.createPipelineLayout(pipelineLayoutCreateInfo);
}

void lune::vulkan::GraphicsPipeline::createPipeline()
{
	const std::vector<vk::PipelineShaderStageCreateInfo> shaderStages =
		{
			reflShaderStage(mVertShader),
			reflShaderStage(mFragShader)};

	const auto [vertAttibues, vertBindings] = reflVertexInput(mVertShader->getReflectModule());

	const auto vertexInputState = vk::PipelineVertexInputStateCreateInfo()
									  .setVertexAttributeDescriptions(vertAttibues)
									  .setVertexBindingDescriptions(vertBindings);

	std::array<vk::PipelineColorBlendAttachmentState, 1> colorBlendAttachments;
	colorBlendAttachments[0] = vk::PipelineColorBlendAttachmentState()
								   .setBlendEnable(VK_FALSE)
								   .setColorWriteMask(
									   vk::ColorComponentFlagBits::eR |
									   vk::ColorComponentFlagBits::eG |
									   vk::ColorComponentFlagBits::eB |
									   vk::ColorComponentFlagBits::eA)
								   .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
								   .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
								   .setColorBlendOp(vk::BlendOp::eAdd)
								   .setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
								   .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
								   .setAlphaBlendOp(vk::BlendOp::eSubtract);

	const std::array<float, 4> colorBlendConstants = {0.F, 0.F, 0.F, 0.F};

	const auto colorBlendingState = vk::PipelineColorBlendStateCreateInfo()
										.setLogicOpEnable(VK_FALSE)
										.setLogicOp(vk::LogicOp::eCopy)
										.setAttachments(colorBlendAttachments)
										.setBlendConstants(colorBlendConstants);

	const auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
										.setTopology(vk::PrimitiveTopology::eTriangleList)
										.setPrimitiveRestartEnable(VK_FALSE);

	const auto rasterizationState = vk::PipelineRasterizationStateCreateInfo()
										.setRasterizerDiscardEnable(VK_FALSE)
										.setPolygonMode(vk::PolygonMode::eFill)
										.setLineWidth(1.0F)
										.setCullMode(vk::CullModeFlagBits::eNone)
										.setFrontFace(vk::FrontFace::eCounterClockwise)
										.setDepthClampEnable(VK_FALSE)
										.setDepthBiasEnable(VK_FALSE)
										.setDepthBiasConstantFactor(0.0F)
										.setDepthBiasSlopeFactor(0.0F)
										.setDepthBiasClamp(0.0F);

	const auto multisamplingState = vk::PipelineMultisampleStateCreateInfo()
										.setSampleShadingEnable(VK_FALSE)
										.setRasterizationSamples(getVulkanConfig().sampleCount)
										.setMinSampleShading(1.0F)
										.setPSampleMask(nullptr)
										.setAlphaToCoverageEnable(VK_TRUE)
										.setAlphaToOneEnable(VK_FALSE);

	const auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo()
									   .setDepthTestEnable(VK_TRUE)
									   .setDepthWriteEnable(VK_TRUE)
									   .setDepthCompareOp(vk::CompareOp::eLess)
									   .setDepthBoundsTestEnable(VK_FALSE)
									   .setMinDepthBounds(0.0F)
									   .setMaxDepthBounds(1.0F)
									   .setStencilTestEnable(VK_TRUE);

	const auto viewportState = vk::PipelineViewportStateCreateInfo()
								   .setViewportCount(1)
								   .setScissorCount(1);

	const auto dynamicStates = std::vector<vk::DynamicState>{vk::DynamicState::eViewport, vk::DynamicState::eScissor, vk::DynamicState::eDepthTestEnable};
	const auto dynamicState = vk::PipelineDynamicStateCreateInfo()
								  .setDynamicStates(dynamicStates);

	const auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
										.setStages(shaderStages)
										.setPVertexInputState(&vertexInputState)
										.setPInputAssemblyState(&inputAssemblyState)
										.setPViewportState(&viewportState)
										.setPDynamicState(&dynamicState)
										.setPRasterizationState(&rasterizationState)
										.setPColorBlendState(&colorBlendingState)
										.setPMultisampleState(&multisamplingState)
										.setPDepthStencilState(&depthStencilState)
										.setLayout(mPipelineLayout)
										.setRenderPass(getVulkanContext().renderPass)
										.setSubpass(0);

	const auto createResult = getVulkanContext().device.createGraphicsPipeline(nullptr, pipelineCreateInfo);
	if (createResult.result != vk::Result::eSuccess)
	{
		LN_LOG(Fatal, Vulkan::Pipeline, "Failed to create pipeline: {}", vk::to_string(createResult.result));
	}
	mPipeline = createResult.value;
}