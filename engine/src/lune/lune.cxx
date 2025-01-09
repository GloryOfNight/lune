#include "lune/lune.hxx"

uint32& lune::getApplicationVersion()
{
	static uint32 version = makeVersion(0, 0, 1);
	return version;
}

std::string& lune::getApplicationName()
{
	static std::string name = "Lune";
	return name;
}
