#include "Asset/Level.h"
#include "Asset/World.h"
#include "FileSystem.h"

std::weak_ptr<Level> Level::LoadAsset(HGUID guid)
{
	return std::weak_ptr<Level>();
}

Level::Level(HString name)
{
	_levelName = name;
}

Level::~Level()
{
}

void Level::Load(World* world)
{
	if (world)
	{
		_world = world;
	}
}

bool Level::UnLevel()
{
	return false;
}

void Level::SaveLevel()
{
	if (!_world)
		return;
	HString assetPath = FileSystem::GetWorldAbsPath() + _world->_worldName;
	HString filePath = assetPath + "/" + _world->_worldName + ".world";

}

void Level::LevelUpdate()
{
}
