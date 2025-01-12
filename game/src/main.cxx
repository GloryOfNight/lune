#include "lune/core/engine.hxx"
#include "lune/game_framework/components/camera.hxx"
#include "lune/game_framework/components/input.hxx"
#include "lune/game_framework/components/move.hxx"
#include "lune/game_framework/components/rotate.hxx"
#include "lune/game_framework/components/sprite.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/systems/camera_system.hxx"
#include "lune/game_framework/systems/input_system.hxx"
#include "lune/game_framework/systems/move_system.hxx"
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
		auto transform = addComponent<lune::TransformComponent>();
		transform->mPosition = lnm::vec3(0.f, 0.f, -10.f);
		//transform->mRotation = lnm::quat(lnm::angleAxis(lnm::radians(180.f), lune::yawAxis));

		addComponent<lune::PerspectiveCameraComponent>();

		addComponent<lune::MoveComponent>();
		addComponent<lune::RotateComponent>();
		auto inputComp = addComponent<lune::InputComponent>();

		inputComp->actions.push_back({"move_front"});
		inputComp->actions.push_back({"move_back"});
		inputComp->actions.push_back({"move_left"});
		inputComp->actions.push_back({"move_right"});
		inputComp->actions.push_back({"move_up"});
		inputComp->actions.push_back({"move_down"});
		inputComp->actions.push_back({"roll_left"});
		inputComp->actions.push_back({"roll_right"});
		inputComp->actions.push_back({"yaw_left"});
		inputComp->actions.push_back({"yaw_right"});
		inputComp->actions.push_back({"pitch_up"});
		inputComp->actions.push_back({"pitch_down"});

		inputComp->actions.push_back({"mouse_left_button"});
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

		addComponent<lune::MoveComponent>();
		addComponent<lune::RotateComponent>();
		auto inputComp = addComponent<lune::InputComponent>();

		// inputComp->actions.push_back({"move_front"});
		// inputComp->actions.push_back({"move_back"});
		// inputComp->actions.push_back({"move_left"});
		// inputComp->actions.push_back({"move_right"});
		// inputComp->actions.push_back({"move_up"});
		// inputComp->actions.push_back({"move_down"});
		// inputComp->actions.push_back({"roll_left"});
		// inputComp->actions.push_back({"roll_right"});
		// inputComp->actions.push_back({"yaw_left"});
		// inputComp->actions.push_back({"yaw_right"});
		// inputComp->actions.push_back({"pitch_up"});
		// inputComp->actions.push_back({"pitch_down"});
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

		auto inputSystem = registerSystem<lune::InputSystem>();
		inputSystem->setWindowId(ln::Engine::get()->getViewWindowId(0));

		registerSystem<lune::MoveSystem>();
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

	uint32 viewId = engine.createWindow("so8", 800, 800);
	//engine.createWindow("so8 - 2", 800, 800);

	engine.addScene(std::make_unique<GameScene>());

	engine.run();

	engine.shutdown();

	return 0;
}