#pragma once
#include "Common.h"
#include "Texture.h"
#include "Pass/PassType.h"
#include "Primitive.h"
#include "Resource/HGuid.h"

#include <unordered_map>
#include <vector>

class Material
{
public:

	Material();
	
	~Material();

	__forceinline static Material* GetDefaultMaterial()
	{
		static HUUID defaultMatGUID;
		if (!_defaultMaterial)
		{
			defaultMatGUID = CreateUUID();
			std::shared_ptr<Material> newMat(new Material());
			newMat->_materialName = "DefaultMaterial";
			newMat->bIsDefaultMaterial = true;
			_allMaterials.emplace(std::make_pair(defaultMatGUID, newMat));
		}
		return _allMaterials[defaultMatGUID].get();
	}

	static HUUID LoadMaterial(const char* materialFilePath);

	__forceinline MaterialPrimitive* GetPrimitive()const { return _primitive.get(); }

private:

	Pass _pass = Pass::OpaquePass;

	HString _materialName = "Unknow material" ;

	std::unique_ptr<MaterialPrimitive> _primitive;

	bool bIsDefaultMaterial;

	static Material* _defaultMaterial;

	static  std::unordered_map<HUUID, std::shared_ptr<Material>> _allMaterials;

};