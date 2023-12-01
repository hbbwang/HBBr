#pragma once
#include "Asset/AssetObject.h"
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "Component/GameObject.h"
#include "./ThirdParty/pugixml/pugixml.hpp"
#include "Asset/HGuid.h"

class Level 
{
	friend class VulkanRenderer;
	friend class GameObject;
	friend class CameraComponent;
	friend class World;
public:

	HBBR_API static std::weak_ptr<Level> LoadAsset(HGUID guid);
	Level() {}
	Level(HString name);
	~Level();

	HBBR_INLINE HBBR_API HString GetLevelName()const { return _levelName; }

	//加载关卡
	HBBR_API void Load(class World* world);

	//释放关卡对象
	HBBR_API bool UnLoad();

	//释放关卡,包括对象和Xml Doc
	HBBR_API bool ResetLevel();

	HBBR_API void SaveLevel();

private:

	void LevelUpdate();

	bool bLoad = false;

	class World* _world = NULL;

	HString _levelName = "NewLevel";

	pugi::xml_document _levelDoc;
};



