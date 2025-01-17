#include "shader_compiler_tool/shader_compiler.hxx"

#define _CRT_SECURE_NO_WARNINGS

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WINDOWS
#define PLATFORM_WINDOWS 1
#else
#define PLATFORM_WINDOWS 0
#endif

std::string lexToString(lune::ShaderCompiler::TargetVkVersion value)
{
	switch (value)
	{
	case lune::ShaderCompiler::TargetVkVersion::Vulkan10:
		return "vulkan1.0";
	case lune::ShaderCompiler::TargetVkVersion::Vulkan11:
		return "vulkan1.1";
	case lune::ShaderCompiler::TargetVkVersion::Vulkan12:
		return "vulkan1.2";
	case lune::ShaderCompiler::TargetVkVersion::Vulkan13:
		return "vulkan1.3";
	default:
		return "vulkan";
	}
}

static bool isStemShaderSource(const std::string_view stem)
{
	if (".vert" == stem)
		return true;
	else if (".frag" == stem)
		return true;
	else if (".geom" == stem)
		return true;
	return false;
}

lune::ShaderCompiler::Result lune::ShaderCompiler::compileShaders(const std::filesystem::path srcShaderDir, const std::filesystem::path binShaderDir, const TargetVkVersion vkVersion)
{
	std::cout << "- - - SHADER COMPILER TOOL - BEGIN" << std::endl;

	const auto beginTime = std::chrono::steady_clock::now();

	namespace fs = std::filesystem;

	std::cout << "shader_src_dir: " << srcShaderDir.generic_string() << std::endl;
	if (!fs::exists(srcShaderDir) || !fs::is_directory(srcShaderDir))
	{
		std::cerr << "provided arg for shaders_src_dir path not exists or not valid" << std::endl;
		return Result::InvalidSrcDir;
	}

	std::cout << "shader_bin_dir: " << binShaderDir.generic_string() << std::endl;
	if ((!fs::exists(binShaderDir) || !fs::is_directory(binShaderDir)))
	{
		try
		{
			if (!fs::create_directory(binShaderDir))
			{
				std::cerr << "provided arg for shaders_bin_dir path not exists and failed to create directory" << std::endl;
				return Result::InvalidBinDir;
			}
		}
		catch (fs::filesystem_error& error)
		{
			std::cerr << error.what() << std::endl;
			return Result::InvalidBinDir;
		}
	}

	std::vector<std::string> totalShaderFiles;
	std::vector<fs::path> needToCompileShaderFiles;

	for (auto& p : fs::recursive_directory_iterator(srcShaderDir))
	{
		if (!p.is_regular_file())
		{
			continue;
		}

		const bool isSourceShaderFile = isStemShaderSource(p.path().extension().string());
		if (isSourceShaderFile)
		{
			const std::string binPathStr = binShaderDir.generic_string() + "/" + p.path().filename().generic_string() + ".spv";
			totalShaderFiles.push_back(binPathStr);

			const fs::path binPath = binPathStr;
			if (fs::exists(binPath) && fs::is_regular_file(binPath))
			{
				const auto lastBinWriteTime = fs::last_write_time(binPath);
				const auto lastCodeWriteTime = fs::last_write_time(p);
				if (lastCodeWriteTime < lastBinWriteTime)
				{
					std::cout << binPathStr << " OK" << std::endl;
					continue;
				}
				else
				{
					std::cout << binPathStr << " OLD" << std::endl;
				}
			}
			else
			{
				std::cout << binPathStr << " MISSING" << std::endl;
			}
			needToCompileShaderFiles.push_back(p.path());
		}
	}

	{
		const std::string vkVerStr = lexToString(vkVersion);
		const std::string exeStem = PLATFORM_WINDOWS ? ".exe" : "";
		for (auto& path : needToCompileShaderFiles)
		{
			const std::string sourceShaderPath = path.generic_string();
			const std::string outputShaderPath = binShaderDir.generic_string() + '/' + path.filename().generic_string() + ".spv";
			const std::string command = "glslc" + exeStem + ' ' + sourceShaderPath + " -o " + outputShaderPath + " --target-env=" + vkVerStr;
			const int result = std::system(command.data());
			if (result == 0)
			{
				std::cout << outputShaderPath << " - COMPILED OK" << std::endl;
			}
			else
			{
				return Result::CompileError;
			}
		}
	}
	const auto endTime = std::chrono::steady_clock::now();

	std::cout << "Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count() / 1000.0 << "ms" << std::endl;

	std::cout << " - - - SHADER COMPILER TOOL - END" << std::endl;
	return Result::Success;
}