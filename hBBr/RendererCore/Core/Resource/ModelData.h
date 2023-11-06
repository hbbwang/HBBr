#pragma once

#include"Common.h"
#include <vector>
#include <map>
#include "HString.h"
#include "VertexFactory.h"
#include "Primitive.h"
#include "HGuid.h"

struct VertexData
{
	glm::vec3 pos;
	glm::vec3 nor;
	glm::vec3 tar;
	glm::vec4 col;
	glm::vec4 uv01;
	glm::vec4 uv23;
};

//FaceData(MaterialData)
struct FaceData	
{
	uint32_t				vertexNum;
	uint32_t				indexNum;
	VertexFactory::VertexInput vertexData;
	char					matName[64];
	char					matPath[512];
};

typedef uint16_t ModelFileDataStructFlags;
typedef enum EModelFileDataStructBits
{
	ModelFileDataStruct_Flags = 0x0001,
	ModelFileDataStruct_MeshInfo = 0x0002,
	ModelFileDataStruct_SubMeshData = 0x0004,
}EModelFileDataStructBits;

struct Vec2FileData
{
	float x = 0, y = 0;
	Vec2FileData() {}
	Vec2FileData(glm::vec2 v)
	{
		x = v.x;
		y = v.y;
	}
};

struct Vec3FileData
{
	float x = 0, y = 0, z = 0;
	Vec3FileData() {}
	Vec3FileData(glm::vec3 v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}
	//把xyz(0-1)压缩到一个32位里(11,11,10),适用于归一化的方向数据
	uint32_t Get32BitData()
	{
		return
			((uint32_t)(x * 0x7FF)) << 21 |
			((uint32_t)(y * 0x7FF)) << 10 |
			((uint32_t)(z * 0x3FF));
	}
	static glm::vec3 From32BitData(uint32_t i)
	{
		glm::vec3 result;
		result.x = ((float)((i >> 21) & 0x7FF)) / (float)0x7FF;
		result.y = ((float)((i >> 10) & 0x7FF)) / (float)0x7FF;
		result.z = ((float)((i) & 0x3FF)) / (float)0x3FF;
		return result;
	}
};

struct Vec4FileData
{
	float x = 0, y = 0, z = 0, w = 0;
	Vec4FileData() {}
	Vec4FileData(glm::vec4 v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
	}
	//把xyzw(0-1)压缩到一个64位里(16,16,16,16),适用于精度稍微比较高一点的数据
	uint64_t Get64BitData()
	{
		return
			((((uint64_t)(x * (float)0xFFDC)) << 48) & 0xFFFF000000000000) |
			((((uint64_t)(y * (float)0xFFDC)) << 32) & 0xFFFF00000000) |
			((((uint64_t)(z * (float)0xFFDC)) << 16) & 0xFFFF0000)|
			((((uint64_t)(w * (float)0xFFDC))) & 0xFFFF);
	}
	static glm::vec4 From64BitData(uint64_t i)
	{
		glm::vec4 result;
		result.x = ((float)((i >> 48) & 0xFFFF)) / (float)0xFFDC;
		result.y = ((float)((i >> 32) & 0xFFFF)) / (float)0xFFDC;
		result.z = ((float)((i >> 16) & 0xFFFF)) / (float)0xFFDC;
		result.w = ((float)(i & 0xFFFF)) / (float)0xFFDC;
		return result;
	}
};

struct Vec4ColorFileData
{
	uint8_t x = 0, y = 0, z = 0, w = 0;
	Vec4ColorFileData() {}
	Vec4ColorFileData(glm::vec4 v)
	{
		x = (uint8_t)(v.x * (0xff));
		y = (uint8_t)(v.y * (0xff));
		z = (uint8_t)(v.z * (0xff));
		w = (uint8_t)(v.w * (0xff));
	}
	// 0 - 1 
	glm::vec4 ToVec4f()
	{
		return glm::vec4(
			(float)x / 0xff,
			(float)y / 0xff,
			(float)z / 0xff,
			(float)w / 0xff
		);
	}
};

struct ModelData
{
	//根据材质区分面
	std::vector<FaceData>		faces;
	ModelFileDataStructFlags	fileFlags = //文件结构枚举,用于识别可能内容不同的模型文件
		ModelFileDataStruct_Flags | ModelFileDataStruct_MeshInfo | ModelFileDataStruct_SubMeshData;
	uint8_t						faceNum = 0;
	glm::vec3					boundingBox_min = glm::vec3(0, 0, 0);
	glm::vec3					boundingBox_max = glm::vec3(0, 0, 0);
	glm::vec4					uv_0_1_abs_max = glm::vec4(0, 0, 0, 0);
	glm::vec4					uv_2_3_abs_max = glm::vec4(0, 0, 0, 0);
	//
	HString						filePath;
	HGUID						guid;
};

class ModelFileStream
{
public:
	
	HBBR_API static ModelData* ImportFbxToMemory(HGUID guid);

	HBBR_API static bool BuildModelPrimitives(ModelData* data ,std::vector<ModelPrimitive*>& prims);

};