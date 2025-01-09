#include "lune/core/engine.hxx"
#include "lune/game_framework/components/isometric_camera.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/systems/camera_system.hxx"
#include "lune/lune.hxx"

#include <iostream>
#include <string>
#include <vector>

class camera_entity : public lune::Entity
{
public:
	camera_entity()
	{
		addComponent<lune::TransformComponent>();
		addComponent<lune::IsometricCameraComponent>();
	}
};

class game_scene : public lune::Scene
{
public:
	game_scene()
	{
		addEntity<camera_entity>();
	}
};

int main(int argc, char** argv)
{
	std::vector<std::string> args(argv, argv + argc);

	ln::getApplicationName() = "so8";
	ln::getApplicationVersion() = ln::makeVersion(1, 0, 0);

	ln::Engine engine = ln::Engine();
	if (!engine.initialize(std::move(args)))
		return 1;

	engine.createWindow("so8", 800, 800);

	engine.addScene(std::make_unique<game_scene>());

	engine.run();

	engine.shutdown();

	return 0;
}