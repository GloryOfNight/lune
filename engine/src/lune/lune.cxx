#include "lune/lune.hxx"

uint32& lune::getApplicationVersion() noexcept
{
	static uint32 version = makeVersion(0, 0, 1);
	return version;
}

std::string& lune::getApplicationName() noexcept
{
	static std::string name = "Lune";
	return name;
}
