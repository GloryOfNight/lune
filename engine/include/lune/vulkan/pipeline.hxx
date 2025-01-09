#pragma once

#include "lune/vulkan/shader.hxx"
#include "lune/vulkan/vulkan_core.hxx"

namespace lune::vulkan
{
	class Pipeline final
	{
	public:
		Pipeline() = default;
		Pipeline(Pipeline&) = delete;
		Pipeline(Pipeline&&) = default;
		~Pipeline() = default;

		static std::shared_ptr<Pipeline> create();

		void init(std::shared_ptr<Shader> vertShader, std::shared_ptr<Shader> fragShader);
		void destroy();

		std::shared_ptr<Shader> getVertShader() const { return mVertShader; }
		std::shared_ptr<Shader> getFragShader() const { return mFragShader; }

		const std::vector<vk::DescriptorSetLayout>& getDescriptorLayouts() const { return mDescriptorSetLayouts; }
		const std::vector<vk::DescriptorPoolSize>& getDescriptorPoolSizes() const { return mPoolSizes; }

		vk::PipelineLayout getPipelineLayout() const { return mPipelineLayout; }
		vk::Pipeline getPipeline() const { return mPipeline; }

		void cmdBind(vk::CommandBuffer commandBuffer);

	private:
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