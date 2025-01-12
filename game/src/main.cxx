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
		auto cam = addComponent<lune::PerspectiveCameraComponent>();

		cam->mPosition = lnm::vec3(0.f, 0.f, 10.f);
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

		auto scarlet1 = addEntity<ScarletSprite>();

		auto scarlet2 = addEntity<ScarletSprite>();
		scarlet2->findComponent<lune::TransformComponent>()->translate(lnm::vec3(2, 0, 0));

		auto scarlet3 = addEntity<ScarletSprite>();
		scarlet3->findComponent<lune::TransformComponent>()->translate(lnm::vec3(-2, 0, 0));

		auto scarlet4 = addEntity<ScarletSprite>();
		scarlet4->findComponent<lune::TransformComponent>()->translate(lnm::vec3(0, 2, 0));

		auto scarlet5 = addEntity<ScarletSprite>();
		scarlet5->findComponent<lune::TransformComponent>()->translate(lnm::vec3(0, -2, 0));

		auto scarlet6 = addEntity<ScarletSprite>();
		scarlet6->findComponent<lune::TransformComponent>()->translate(lnm::vec3(2, 2, 2));

		auto scarlet7 = addEntity<ScarletSprite>();
		scarlet7->findComponent<lune::TransformComponent>()->translate(lnm::vec3(-2, -2, -2));

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
	engine.createWindow("so8 - 2", 800, 800);

	engine.addScene(std::make_unique<GameScene>());

	engine.run();

	engine.shutdown();

	return 0;
}