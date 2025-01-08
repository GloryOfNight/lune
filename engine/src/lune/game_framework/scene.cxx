#include "game_framework/scene.hxx"

#include "game_framework/systems/render_system.hxx"

#include "scene.hxx"

void lune::scene::update(double deltaTime)
{
	for (auto s : mSystems)
	{
		s->update(mEntities, deltaTime);
	}
}

void lune::scene::render()
{
	for (auto s : mSystems)
	{
		if (auto rs = dynamic_cast<render_system*>(s.get()); rs)
		{
			rs->render(mEntities);
		}
	}
}
