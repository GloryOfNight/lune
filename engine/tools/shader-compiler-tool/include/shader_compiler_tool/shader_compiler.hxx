#pragma once

#include <filesystem>
#include <string_view>

namespace lune
{
	struct ShaderCompiler
	{
		enum class TargetVkVersion : uint8_t
		{
			VulkanDefault = 0,
			Vulkan10 = 1,
			Vulkan11 = 2,
			Vulkan12 = 3,
			Vulkan13 = 4
		};
		enum class Result : uint8_t
		{
			Success = 0,
			InvalidSrcDir = 1,
			InvalidBinDir = 2,
			CompileError = 3
		};
		static Result compileShaders(const std::filesystem::path srcShaderDir, const std::filesystem::path binShaderDir, const TargetVkVersion vkVersion);
	};
} // namespace lune
