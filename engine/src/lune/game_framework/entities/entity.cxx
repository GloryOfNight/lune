#include "lune/game_framework/entities/entity.hxx"

static uint64 entityIdCounter{};

lune::EntityBase::EntityBase()
	: mId{++entityIdCounter}
	, mComponents{}
{
}