#pragma once

#include "lune/vulkan/shader.hxx"
#include "lune/vulkan/vulkan_core.hxx"

namespace lune::vulkan
{
	class GraphicsPipeline final
	{
	public:
		GraphicsPipeline() = default;
		GraphicsPipeline(GraphicsPipeline&) = delete;
		GraphicsPipeline(GraphicsPipeline&&) = default;
		~GraphicsPipeline();

		static SharedGraphicsPipeline create(std::shared_ptr<Shader> vertShader, std::shared_ptr<Shader> fragShader);

		std::shared_ptr<Shader> getVertShader() const { return mVertShader; }
		std::shared_ptr<Shader> getFragShader() const { return mFragShader; }

		const std::vector<vk::DescriptorSetLayout>& getDescriptorLayouts() const { return mDescriptorSetLayouts; }
		const std::vector<vk::DescriptorPoolSize>& getDescriptorPoolSizes() const { return mPoolSizes; }

		vk::PipelineLayout getPipelineLayout() const { return mPipelineLayout; }
		vk::Pipeline getPipeline() const { return mPipeline; }

		void cmdBind(vk::CommandBuffer commandBuffer);

	private:
		void init(std::shared_ptr<Shader> vertShader, std::shared_ptr<Shader> fragShader);

		void createDescriptorLayoutsAndPoolSizes();
		void createPipelineLayout();
		void createPipeline();

		std::shared_ptr<Shader> mVertShader{};
		std::shared_ptr<Shader> mFragShader{};

		std::vector<vk::DescriptorSetLayout> mDescriptorSetLayouts{};
		std::vector<vk::DescriptorPoolSize> mPoolSizes{};

		vk::PipelineLayout mPipelineLayout{};
		vk::Pipeline mPipeline{};
	};
} // namespace lune::vulkan