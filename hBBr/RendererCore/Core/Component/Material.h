#pragma once

#include <vector>
#include "Texture.h"
#include "Pass/PassType.h"
#include "Primitive.h"

class Material
{
public:

	Material();
	
	~Material();

	__forceinline static Material* GetDefaultMaterial()
	{
		if (!_defaultMaterial)
		{
			_defaultMaterial.reset(new Material());
			_defaultMaterial->_materialName = "DefaultMaterial";
			_defaultMaterial->bIsDefaultMaterial = true;
		}
		return _defaultMaterial.get();
	}

	__forceinline MaterialPrimitive* GetPrimitive()const { return _primitive.get(); }

private:

	Pass _pass = Pass::OpaquePass;

	HString _materialName = "Unknow material" ;

	std::unique_ptr<MaterialPrimitive> _primitive;

	bool bIsDefaultMaterial;

	static std::shared_ptr<Material> _defaultMaterial;

};