#pragma once
#include "Asset/AssetObject.h"
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "Component/GameObject.h"
#include "./ThirdParty/pugixml/pugixml.hpp"
#include "Asset/HGuid.h"

class Level : public AssetObject
{
	friend class VulkanRenderer;
	friend class GameObject;
	friend class CameraComponent;
	friend class World;
public:

	HBBR_API static std::weak_ptr<Level> LoadAsset(HGUID guid);
	~Level();

	//加载关卡
	HBBR_API void Load(class World* world);

	//释放关卡
	HBBR_API bool UnLevel();

	HBBR_API void SaveLevel();

private:

	void LevelUpdate();

	bool bLoad = false;

	class World* _world = NULL;
};



