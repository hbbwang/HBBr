#pragma once
#include "Common.h"
#include "Texture.h"
#include "Pass/PassType.h"
#include "Primitive.h"
#include "Resource/HGuid.h"
#include "XMLStream.h"

#include <unordered_map>
#include <vector>

struct MaterialParameterInfo
{
	HString name, type, ui;
	uint32_t beginPos;
};

class Material
{
public:
	Material(bool bDefault = false);
	
	~Material();

	__forceinline static Material* GetDefaultMaterial()
	{
		static HUUID defaultMatGUID;
		if (!_defaultMaterial)
		{
			defaultMatGUID = CreateUUID();
			std::unique_ptr<Material> newMat(new Material(true));
			newMat->_materialName = "DefaultMaterial";
			_allMaterials.emplace(std::make_pair(defaultMatGUID, std::move(newMat)));
		}
		return _allMaterials[defaultMatGUID].get();
	}

	__forceinline static Material* GetErrorMaterial()
	{
		static HUUID errorMatGUID;
		if (!_defaultMaterial)
		{
			errorMatGUID = CreateUUID();
			std::unique_ptr<Material> newMat(new Material(true));
			newMat->_materialName = "ErrorMaterial";
			_allMaterials.emplace(std::make_pair(errorMatGUID, std::move(newMat)));
		}
		return _allMaterials[errorMatGUID].get();
	}

	static Material* LoadMaterial(HString materialFilePath);

	__forceinline MaterialPrimitive* GetPrimitive()const { return _primitive.get(); }

private:

	HString _materialName = "Unknow material" ;

	std::unique_ptr<MaterialPrimitive> _primitive;

	HUUID _uuid;

	std::vector<MaterialParameterInfo> _paramterInfos;

	static Material* _defaultMaterial;

	static Material* _errorMaterial;

	static  std::unordered_map<HUUID, std::unique_ptr<Material>> _allMaterials;

};