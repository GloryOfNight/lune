#include "lune/core/assets.hxx"
#include "lune/core/engine.hxx"
#include "lune/core/gltf.hxx"
#include "lune/game_framework/components/camera.hxx"
#include "lune/game_framework/components/input.hxx"
#include "lune/game_framework/components/move.hxx"
#include "lune/game_framework/components/rotate.hxx"
#include "lune/game_framework/components/skybox.hxx"
#include "lune/game_framework/components/sprite.hxx"
#include "lune/game_framework/components/transform.hxx"
#include "lune/game_framework/systems/camera_system.hxx"
#include "lune/game_framework/systems/gizmo_system.hxx"
#include "lune/game_framework/systems/input_system.hxx"
#include "lune/game_framework/systems/mesh_render_system.hxx"
#include "lune/game_framework/systems/move_system.hxx"
#include "lune/game_framework/systems/skybox_system.hxx"
#include "lune/game_framework/systems/sprite_render_system.hxx"
#include "lune/lune.hxx"

#include <imgui.h>
#include <iostream>
#include <string>
#include <vector>

class DebugSystem : public lune::SystemBase
{
public:
	virtual void update(lune::Scene* scene, double deltaTime) override
	{
		auto eIds = scene->getComponentEntities<lune::SpriteComponent>();
		for (auto eId : eIds)
		{
			auto entity = scene->findEntity(eId);
			auto transformComp = entity->findComponent<lune::TransformComponent>();

			// if (ImGui::GetCurrentContext())
			// {
			// 	lnm::vec3 e = lnm::eulerAngles(transformComp->mOrientation);
			// 	auto q = transformComp->mOrientation;
			// 	ImGui::Begin("camera");
			// 	ImGui::SliderFloat4("quat", &transformComp->mOrientation.x, -1.f, 1.f);
			// 	ImGui::InputFloat3("euler", &e.x);
			// 	ImGui::End();

			// 	transformComp->mOrientation = lnm::normalize(transformComp->mOrientation);
			// 	return;
			// }
		}
	}
};

class CameraEntity : public lune::EntityBase
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

class SkyboxEntity : public lune::EntityBase
{
public:
	SkyboxEntity()
	{
		auto skybox = addComponent<lune::SkyboxComponent>();
		skybox->imageName = "lune::skyboxes::sea";
	}
};

class ScarletSprite : public lune::EntityBase
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
		addEntity<SkyboxEntity>();

		auto scarlet1 = addEntity<ScarletSprite>();
		scarlet1->findComponent<lune::TransformComponent>()->translate(lnm::vec3(0, 0, 0));

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
		registerSystem<lune::SkyboxSystem>();
		registerSystem<lune::SpriteRenderSystem>();
		registerSystem<DebugSystem>();
		registerSystem<lune::GizmoSystem>();
		registerSystem<lune::MeshRenderSystem>();

		auto inputSystem = registerSystem<lune::InputSystem>();
		inputSystem->setWindowId(ln::Engine::get()->getViewWindowId(0));

		registerSystem<lune::MoveSystem>();
	}
};

int main(int argc, char** argv)
{
	std::vector<std::string> args(argv, argv + argc);

	ln::getApplicationName() = "game";
	ln::getApplicationVersion() = ln::makeVersion(1, 0, 0);

	ln::Engine engine = ln::Engine();
	if (!engine.initialize(std::move(args)))
		return 1;

	uint32 viewId = engine.createWindow("so8", 800, 800);
	//engine.createWindow("so8 - 2", 800, 800);

	auto scene = engine.addScene(std::make_unique<GameScene>());

	lune::gltf::loadInScene(*lune::EngineAssetPath("viking_room/scene.gltf"), "viking_room", scene);
	lune::gltf::loadInScene(*lune::EngineAssetPath("mi-24d/scene.gltf"), "mi-24d", scene);

	engine.run();

	engine.shutdown();

	return 0;
}