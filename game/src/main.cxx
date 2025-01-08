#include "lune/engine.hxx"
#include "lune/lune.hxx"

#include <iostream>
#include <string>
#include <vector>

struct game_componentA : public lune::component
{
	uint64 gameA;
};

struct game_componentB : public lune::component
{
	uint64 gameB;
};

class game_entity : public lune::entity
{
public:
	game_entity()
	{
		addComponent<game_componentA>();
		attachComponent(std::make_unique<game_componentB>());
	}

	virtual void update(double DeltaTime) override {}
};

class game_scene : public lune::scene
{
public:
	game_scene()
	{
		addEntity<game_entity>();
	}
};

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