#pragma once
#include "Common.h"
#include "Texture.h"
#include "Pass/PassType.h"
#include "Primitive.h"
#include "Resource/HGuid.h"
#include "XMLStream.h"
#include "Shader.h"

#include <unordered_map>
#include <vector>

struct MaterialParameterInfo
{
	MPType type;
	HString name, ui;
	uint32_t beginPos;
};

class Material
{
public:
	Material(bool bDefault = false);
	
	~Material();

	HBBR_API __forceinline static Material* GetDefaultMaterial()
	{
		static HGUID defaultMatGUID;
		if (!_defaultMaterial)
		{
			defaultMatGUID = CreateGUID();
			std::unique_ptr<Material> newMat(new Material(true));
			newMat->_materialName = "DefaultMaterial";
			_allMaterials.emplace(std::make_pair(defaultMatGUID, std::move(newMat)));
		}
		return _allMaterials[defaultMatGUID].get();
	}

	HBBR_API __forceinline static Material* GetErrorMaterial()
	{
		static HGUID errorMatGUID;
		if (!_defaultMaterial)
		{
			errorMatGUID = CreateGUID();
			std::unique_ptr<Material> newMat(new Material(true));
			newMat->_materialName = "ErrorMaterial";
			_allMaterials.emplace(std::make_pair(errorMatGUID, std::move(newMat)));
		}
		return _allMaterials[errorMatGUID].get();
	}

	HBBR_API static Material* LoadMaterial(HString materialFilePath);

	HBBR_API static Material* CreateMaterial(HString newMatFilePath);

	HBBR_API void ResetMaterialGUID();

	HBBR_API void UpdateReference(HGUID newGUID);

	__forceinline MaterialPrimitive* GetPrimitive()const { return _primitive.get(); }

private:

	HString _materialName = "Unknow material" ;

	std::unique_ptr<MaterialPrimitive> _primitive;

	HGUID _uuid;

	std::vector<MaterialParameterInfo> _paramterInfos;

	static Material* _defaultMaterial;

	static Material* _errorMaterial;

	static  std::unordered_map<HGUID, std::unique_ptr<Material>> _allMaterials;

};