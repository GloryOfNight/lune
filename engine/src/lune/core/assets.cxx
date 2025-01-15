#include "lune/core/assets.hxx"

#include "lune/core/log.hxx"

#include <filesystem>

std::filesystem::path& lune::getEngineAssetsRootDir()
{
	namespace fs = std::filesystem;
	static fs::path path = fs::current_path() / "engine" / "assets";

	if (!fs::is_directory(path)) [[unlikely]]
	{
		if (fs::path checkPath{"assets/engine"}; fs::is_directory(checkPath))
			path = checkPath;
		else if (fs::path checkPath{"../engine/assets"}; fs::is_directory(checkPath))
			path = checkPath;
		else if (fs::path checkPath{"../../engine/assets"}; fs::is_directory(checkPath))
			path = checkPath;
		else
			LN_LOG(Fatal, Engine::Assets, "Failed to find engine assets root directory, cwd: {}", fs::current_path().generic_string());
		path = fs::absolute(path);
	}
	return path;
}

lune::EngineAssetPath::EngineAssetPath(std::string_view relativeAssetPath)
	: mPath{getEngineAssetsRootDir() / std::filesystem::path(relativeAssetPath.data())}
{
	if (!std::filesystem::is_regular_file(mPath)) [[unlikely]]
		LN_LOG(Warning, Engine::Assets, "Assets missing: {}", mPath.generic_string());
}

lune::EngineShadersSourceDir::EngineShadersSourceDir()
	: EngineAssetPath()
{
	mPath = getEngineAssetsRootDir() / "shaders" / "src";
}

lune::EngineShadersBinaryDir::EngineShadersBinaryDir()
	: EngineAssetPath()
{
	mPath = getEngineAssetsRootDir() / "shaders" / "bin";
}

lune::EngineShaderPath::EngineShaderPath(std::string_view relativeAssetPath)
	: EngineAssetPath()
{
	mPath = std::filesystem::path(*EngineShadersBinaryDir()) / std::filesystem::path(relativeAssetPath.data());
	if (!std::filesystem::is_regular_file(mPath)) [[unlikely]]
		LN_LOG(Warning, Engine::Assets, "Shader missing: {}", mPath.generic_string());
}
