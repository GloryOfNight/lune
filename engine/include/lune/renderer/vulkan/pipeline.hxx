#pragma once

#include "shader.hxx"
#include "vulkan_core.hxx"

namespace lune::vulkan
{
	class pipeline final
	{
	public:
		pipeline() = default;
		pipeline(pipeline&) = delete;
		pipeline(pipeline&&) = default;
		~pipeline() = default;

		static std::shared_ptr<pipeline> create();

		void init(std::shared_ptr<shader> vertShader, std::shared_ptr<shader> fragShader);
		void destroy();

		std::shared_ptr<shader> getVertShader() const { return mVertShader; }
		std::shared_ptr<shader> getFragShader() const { return mFragShader; }

		const std::vector<vk::DescriptorSetLayout>& getDescriptorLayouts() const { return mDescriptorSetLayouts; }
		const std::vector<vk::DescriptorPoolSize>& getDescriptorPoolSizes() const { return mPoolSizes; }

		vk::PipelineLayout getPipelineLayout() const { return mPipelineLayout; }
		vk::Pipeline getPipeline() const { return mPipeline; }

		void cmdBind(vk::CommandBuffer commandBuffer);

	private:
		void createDescriptorLayoutsAndPoolSizes();
		void createPipelineLayout();
		void createPipeline();

		std::shared_ptr<shader> mVertShader{};
		std::shared_ptr<shader> mFragShader{};

		std::vector<vk::DescriptorSetLayout> mDescriptorSetLayouts{};
		std::vector<vk::DescriptorPoolSize> mPoolSizes{};

		vk::PipelineLayout mPipelineLayout{};
		vk::Pipeline mPipeline{};
	};
} // namespace lune::vulkan