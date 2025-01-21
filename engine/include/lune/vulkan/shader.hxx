#pragma once

#include "vulkan/vulkan.hpp"

#include "spirv_reflect.h"
#include "vulkan_core.hxx"

#include <filesystem>

namespace lune::vulkan
{
	class Shader final
	{
	public:
		Shader() = default;
		Shader(Shader&) = delete;
		Shader(Shader&&) = default;
		~Shader();

		static SharedShader create(const std::filesystem::path spvPath);

		vk::ShaderModule getShaderModule() const { return mShaderModule; }

		SpvReflectShaderModule getReflectModule() const { return mReflectModule; }

	private:
		bool init(const std::string& spvCode);

		vk::ShaderModule mShaderModule{};

		SpvReflectShaderModule mReflectModule{};
	};
} // namespace lune::vulkan