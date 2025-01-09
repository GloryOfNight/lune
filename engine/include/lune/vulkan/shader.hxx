#pragma once

#include "vulkan/vulkan.hpp"

#include "spirv_reflect.h"

namespace lune::vulkan
{
	class Shader final
	{
	public:
		Shader() = default;
		Shader(Shader&) = delete;
		Shader(Shader&&) = default;
		~Shader() = default;

		static std::shared_ptr<Shader> create();

		void init(const std::string_view spvPath);
		void destroy();

		vk::ShaderModule getShaderModule() const { return mShaderModule; }

		SpvReflectShaderModule getReflectModule() const { return mReflectModule; }

	private:
		vk::ShaderModule mShaderModule{};

		SpvReflectShaderModule mReflectModule{};
	};
} // namespace lune::vulkan