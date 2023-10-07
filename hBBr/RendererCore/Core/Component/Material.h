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

#define DefaultMaterialGuid "61A147FF-32BD-48EC-B523-57BC75EB16BA"

struct MaterialParameterInfo
{
	MPType type;
	HString name, ui;
	uint32_t beginPos;
};

class Material
{
public:
	Material();
	
	~Material();

	HBBR_API static Material* LoadMaterial(HGUID guid);

	HBBR_API static Material* CreateMaterial(HString newMatFilePath);

	HBBR_API HBBR_INLINE MaterialPrimitive* GetPrimitive()const { return _primitive.get(); }

	HBBR_API HBBR_INLINE HGUID GetGUID()const { return _guid; }

	HBBR_API HBBR_INLINE static Material* GetDefaultMaterial()
	{	
		return LoadMaterial(HGUID(DefaultMaterialGuid));
	}


private:

	HString _materialName = "Unknow material" ;

	std::unique_ptr<MaterialPrimitive> _primitive;

	HGUID _guid;

	std::vector<MaterialParameterInfo> _paramterInfos;
};