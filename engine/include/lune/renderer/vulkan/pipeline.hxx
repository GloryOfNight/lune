#pragma once

#include "vulkan/vulkan.hpp"

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
		void createPipeline();
		void createPipelineLayout();

		std::shared_ptr<shader> mVertShader{};
		std::shared_ptr<shader> mFragShader{};

		vk::Pipeline mPipeline{};
	};
} // namespace lune::vulkan