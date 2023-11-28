#include "Asset/Level.h"
#include "Asset/World.h"

std::weak_ptr<Level> Level::LoadAsset(HGUID guid)
{
	return std::weak_ptr<Level>();
}

Level::~Level()
{
}

void Level::Load(World* world)
{
	if (world)
	{
		_world = world;
		_renderer = world->_renderer;

	}
}

bool Level::UnLevel()
{
	return false;
}

void Level::LevelUpdate()
{
}
