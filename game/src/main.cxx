#include "lune/engine.hxx"
#include "lune/lune.hxx"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
	std::vector<std::string> args(argv, argv + argc);

	ln::getApplicationName() = "so8";
	ln::getApplicationVersion() = ln::makeVersion(1, 0, 0);

	ln::engine engine = ln::engine();
	if (!engine.initialize(std::move(args)))
		return 1;

	engine.createWindow("so8", 800, 600);

	engine.run();

	engine.shutdown();

	return 0;
}