#include "lune/vulkan/shader.hxx"

#include "lune/core/log.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

#include <filesystem>
#include <fstream>

std::string readFile(const std::filesystem::path& path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		return std::string{};

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::string fileContent(fileSize, '\0');

	file.seekg(0);
	file.read(fileContent.data(), fileSize);

	file.close();

	return fileContent;
}

lune::vulkan::Shader::~Shader()
{
	spvReflectDestroyShaderModule(&mReflectModule);
	const auto cleanShaderModuleLam = [shaderModule = mShaderModule]() -> bool
	{
		getVulkanContext().device.destroyShaderModule(shaderModule);
		return true;
	};
	getVulkanDeleteQueue().push(cleanShaderModuleLam);
}

lune::vulkan::SharedShader lune::vulkan::Shader::create(const std::filesystem::path spvPath)
{
	if (!std::filesystem::is_regular_file(spvPath)) [[unlikely]]
	{
		LN_LOG(Error, Vulkan::Shader, "Not a file: {}", spvPath.generic_string());
		return nullptr;
	}

	const std::string spvCode = readFile(spvPath);
	if (spvCode.size() % 4 != 0) [[unlikely]]
	{
		LN_LOG(Error, Vulkan::Shader, "Binary not power of 4: {}", spvPath.generic_string());
		return nullptr;
	}

	auto newShader = std::make_shared<Shader>();
	const bool wasInit = newShader->init(spvCode);
	if (wasInit) [[likely]]
		return std::move(newShader);
	return nullptr;
}

bool lune::vulkan::Shader::init(const std::string& spvCode)
{
	const auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo({}, spvCode.size(), reinterpret_cast<const uint32_t*>(spvCode.data()));
	mShaderModule = getVulkanContext().device.createShaderModule(shaderModuleCreateInfo);
	if (!mShaderModule)
	{
		LN_LOG(Fatal, Vulkan::Shader, "Failed to create shader module");
		return false;
	}

	const SpvReflectResult result = spvReflectCreateShaderModule(spvCode.size(), spvCode.data(), &mReflectModule);
	if (result != SPV_REFLECT_RESULT_SUCCESS)
	{
		LN_LOG(Fatal, Vulkan::Shader, "Failed to reflect shader module");

		getVulkanContext().device.destroyShaderModule(mShaderModule);
		mShaderModule = vk::ShaderModule();

		return false;
	}
	return true;
}