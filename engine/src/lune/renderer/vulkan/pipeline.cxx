#include "pipeline.hxx"

#include "log.hxx"
#include "vulkan_subsystem.hxx"

#include <utility>

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

vk::PipelineShaderStageCreateInfo reflShaderStage(const std::shared_ptr<lune::vulkan::shader>& shader)
{
	return vk::PipelineShaderStageCreateInfo()
		.setModule(shader->getShaderModule())
		.setStage(static_cast<vk::ShaderStageFlagBits>(shader->getReflectModule().shader_stage))
		.setPName(shader->getReflectModule().entry_point_name);
}

std::unique_ptr<lune::vulkan::pipeline> lune::vulkan::pipeline::create()
{
	return std::make_unique<pipeline>();
}

void lune::vulkan::pipeline::init(std::shared_ptr<shader> vertShader, std::shared_ptr<shader> fragShader)
{
	mVertShader = vertShader;
	mFragShader = fragShader;

	if (!mVertShader || !mFragShader)
	{
		LN_LOG(Fatal, Vulkan::Pipeline, "Failed to initialize pipeline: missing shader(s)");
		return;
	}

	createPipeline();
}

void lune::vulkan::pipeline::destroy()
{
}

void lune::vulkan::pipeline::createPipeline()
{
}