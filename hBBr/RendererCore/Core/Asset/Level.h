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
	void Load(class World* world);

	//释放关卡
	bool UnLevel();

private:

	void LevelUpdate();

	bool bLoad = false;

	class World* _world = NULL;

	class VulkanRenderer* _renderer = NULL;
};



