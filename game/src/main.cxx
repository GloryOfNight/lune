#include "lune/core/engine.hxx"
#include "lune/game_framework/components/camera.hxx"
#include "lune/game_framework/components/sprite.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/systems/camera_system.hxx"
#include "lune/game_framework/systems/sprite_render_system.hxx"
#include "lune/lune.hxx"

#include <iostream>
#include <string>
#include <vector>

class CameraEntity : public lune::Entity
{
public:
	CameraEntity()
	{
		addComponent<lune::TransformComponent>();
		addComponent<lune::PerspectiveCameraComponent>();
	}
};

class ScarletSprite : public lune::Entity
{
public:
	ScarletSprite()
	{
		addComponent<lune::TransformComponent>();
		auto sprite = addComponent<lune::SpriteComponent>();
		sprite->imageName = "lune::scarlet";
	}
};

class GameScene : public lune::Scene
{
public:
	GameScene()
	{
		addEntity<CameraEntity>();
		addEntity<ScarletSprite>();

		registerSystem<lune::CameraSystem>();
		registerSystem<lune::SpriteRenderSystem>();
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

	engine.addScene(std::make_unique<GameScene>());

	engine.run();

	engine.shutdown();

	return 0;
}