#include "lune/vulkan/shader.hxx"

#include "lune/core/log.hxx"
#include "lune/vulkan/vulkan_subsystem.hxx"

#include <fstream>

std::string readFile(const std::string_view path)
{
	std::ifstream file(path.data(), std::ios::ate | std::ios::binary);

	if (!file.is_open())
		return std::string{};

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::string fileContent(fileSize, '\0');

	file.seekg(0);
	file.read(fileContent.data(), fileSize);

	file.close();

	return fileContent;
}

std::shared_ptr<lune::vulkan::Shader> lune::vulkan::Shader::create()
{
	return std::make_shared<Shader>();
}

void lune::vulkan::Shader::init(const std::string_view spvPath)
{
	const std::string spvCode = readFile(spvPath);

	if (spvCode.empty())
	{
		LN_LOG(Fatal, Vulkan::Shader, "Failed to read shader code: {}", spvPath);
		return;
	}

	const auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo({}, spvCode.size(), reinterpret_cast<const uint32_t*>(spvCode.data()));
	mShaderModule = getVulkanContext().device.createShaderModule(shaderModuleCreateInfo);
	if (!mShaderModule)
	{
		LN_LOG(Fatal, Vulkan::Shader, "Failed to create shader module: {}", spvPath);
		return;
	}

	const SpvReflectResult result = spvReflectCreateShaderModule(spvCode.size(), spvCode.data(), &mReflectModule);
	if (result != SPV_REFLECT_RESULT_SUCCESS)
	{
		LN_LOG(Fatal, Vulkan::Shader, "Failed to reflect shader module: {}", spvPath);
		return;
	}
}

void lune::vulkan::Shader::destroy()
{
	if (mShaderModule)
	{
		getVulkanContext().device.destroyShaderModule(mShaderModule);
	}
	new (this) Shader();
}
