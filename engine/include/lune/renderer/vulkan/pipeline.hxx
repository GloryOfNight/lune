#pragma once

#include "vulkan/vulkan.hpp"

#include "lune.hxx"
#include "shader.hxx"

namespace lune::vulkan
{
	class pipeline final
	{
	public:
		pipeline() = default;
		pipeline(pipeline&) = delete;
		pipeline(pipeline&&) = default;
		~pipeline() = default;

		static std::unique_ptr<pipeline> create();

		void init(std::shared_ptr<shader> vertShader, std::shared_ptr<shader> fragShader);
		void destroy();

		vk::Pipeline getPipeline() const { return mPipeline; }

	private:
		void createDescriptorPool();

		void createPipelineLayout();
		void createPipeline();

		std::shared_ptr<shader> mVertShader{};
		std::shared_ptr<shader> mFragShader{};

		std::vector<vk::DescriptorSetLayout> mDescriptorSetLayouts{};
		vk::DescriptorPool mDescriptorPool{};
		uint32 mDescriptorPoolMaxSets{2};

		vk::PipelineLayout mPipelineLayout{};
		vk::Pipeline mPipeline{};
	};
} // namespace lune::vulkan