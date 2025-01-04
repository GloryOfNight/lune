#include "lune/engine.hxx"
#include "lune/lune.hxx"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
	std::vector<std::string> args(argv, argv + argc);

	ln::engine engine = ln::engine();
	if (!engine.initialize(std::move(args)))
		return 1;

	engine.run();

	engine.shutdown();

	return 0;
}