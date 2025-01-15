#include "lune/game_framework/entities/entity.hxx"

static uint64 entityIdCounter{};

lune::Entity::Entity()
	: mId{++entityIdCounter}
	, mComponents{}
{
}