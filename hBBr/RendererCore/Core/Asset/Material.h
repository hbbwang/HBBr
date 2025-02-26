﻿#pragma once
#include "Common.h"
#include "Primitive.h"
#include "Asset/HGuid.h"
#include "AssetObject.h"
#include <unordered_map>
#include <vector>

#define DefaultMaterialGuid "b51e2e9a-0985-75e8-6138-fa95efcbab57"

class Material :public AssetObject
{
	friend class PipelineIndex;
public:
	Material();
	
	~Material();

	HBBR_API static std::weak_ptr<Material> GetDefaultMaterial(); 

	HBBR_API static std::weak_ptr<Material> GetErrorMaterial();

	HBBR_API static std::shared_ptr<Material> LoadAsset(HGUID guid);

#if IS_EDITOR

	HBBR_API void SaveAsset(std::string path)override;

	HBBR_API static  std::weak_ptr<AssetInfoBase> CreateMaterial(std::string repository,std::string virtualPath);

#endif

	HBBR_API HBBR_INLINE MaterialPrimitive* GetPrimitive()const { return _primitive.get(); }

	HBBR_INLINE std::vector<std::shared_ptr<class Texture2D>> GetTextures() {
		return _primitive->GetTextures();
	}

private:

	std::string _materialName = "Unknow material" ;

	std::unique_ptr<MaterialPrimitive> _primitive;

	nlohmann::json ToJson()override;

	void FromJson() override;

};