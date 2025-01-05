#pragma once

#include "vulkan/vulkan.hpp"

#include "spirv_reflect.h"

namespace lune::vulkan
{
	class shader final
	{
	public:
		shader() = default;
		shader(shader&) = delete;
		shader(shader&&) = default;
		~shader() = default;

		static std::shared_ptr<shader> create();

		void init(const std::string_view spvPath);
		void destroy();

		vk::ShaderModule getShaderModule() const { return mShaderModule; }

		SpvReflectShaderModule getReflectModule() const { return mReflectModule; }

	private:
		vk::ShaderModule mShaderModule{};

		SpvReflectShaderModule mReflectModule{};
	};
} // namespace lune::vulkan