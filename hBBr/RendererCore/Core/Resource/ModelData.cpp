#include "ModelData.h"

#ifdef NDEBUG
//非Debug我们直接用静态库打入exe内。
#pragma comment(lib,"assimp/lib/Static/Release/assimp-vc142-mt.lib")
#pragma comment(lib,"assimp/lib/Static/Release/zlibstatic.lib") 
#else
//Debug使用dll进行调试
#pragma comment(lib,"assimp/lib/assimp-vc142-mt.lib") 
#endif
#include "ConsoleDebug.h"
#include "FileSystem.h"

#include <ThirdParty/assimp/include/assimp/Importer.hpp>      // C++ importer interface
#include <ThirdParty/assimp/include/assimp/scene.h>			// C++ importer interface
#include <ThirdParty/assimp/include/assimp/postprocess.h>     // Post processing flags

#include "ContentManager.h"

ModelData* ModelFileStream::ImportFbxToMemory(HGUID guid)
{
	const auto modelAssets = ContentManager::Get()->GetAssets(AssetType::Model);
	HString guidStr = GUIDToString(guid);
	//从内容管理器查找资产
	auto it = modelAssets.find(guid);
	{	
		if (it == modelAssets.end())
		{
			MessageOut(HString("Can not find [" + guidStr + "] model in content manager.").c_str(), false, false, "255,255,0");
			return NULL;
		}
	}
	auto dataPtr = reinterpret_cast<AssetInfo<ModelData>*>(it->second);
	if (dataPtr->IsAssetLoad())
	{
		return dataPtr->GetData();
	}
	//获取实际路径
	HString filePath = FileSystem::GetContentAbsPath() + it->second->relativePath + guidStr + ".fbx";
	filePath.CorrectionPath();
	if (!FileSystem::FileExist(filePath.c_str()))
	{
		return NULL;
	}
	//导入fbx文件
#if IS_EDITOR
	ConsoleDebug::print_endl("Ready import fbx model :" + filePath, "255,255,255");
#endif
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath.c_str(),
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_GenBoundingBoxes
		//Direct 3D flags
		|aiProcess_ConvertToLeftHanded
	);
	if (!scene)
	{
		MessageOut(importer.GetErrorString(), false, true, "255,255,0");
		return NULL;
	}
	if (!scene->HasMaterials())
	{
		MessageOut("Error,cannot find materials in this fbx file.Import failed.", false, true, "255,255,0");
		return NULL;
	}
	if (!scene->HasMeshes())
	{
		MessageOut("Error,cannot find meshes in this fbx file.Import failed.", false, true, "255,255,0");
		return NULL;
	}

	auto modelData = std::make_unique<ModelData>();
	modelData->guid = guid;
	glm::vec3 boundingBox_min = glm::vec3(0, 0, 0);
	glm::vec3 boundingBox_max = glm::vec3(0, 0, 0);
	for (unsigned int nm = 0; nm < scene->mNumMeshes; nm++)
	{
		aiMesh* mesh = scene->mMeshes[nm];
#if IS_EDITOR
		ConsoleDebug::print_endl("Found meshes successful.\nThere are " + HString::FromUInt(mesh->mNumVertices) + " vertices have been found.", "160,160,160");
#endif
		//
		FaceData newData = {};
		newData.vertexNum = mesh->mNumVertices;
		modelData->faceNum += 1;
		//
		VertexFactory::VertexInput vertex{};
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			//VertexData vertex{};
			glm::vec3 vec3 = glm::vec3(0);
			glm::vec4 vec4 = glm::vec4(0);
			//
			vec3.x = mesh->mVertices[i].x;
			vec3.y = mesh->mVertices[i].y;
			vec3.z = mesh->mVertices[i].z;
			vertex.pos.push_back(vec3);
			{//计算Bounding Box大小
				boundingBox_min.x = boundingBox_min.x >= vec3.x ? vec3.x : boundingBox_min.x;
				boundingBox_min.y = boundingBox_min.y >= vec3.y ? vec3.y : boundingBox_min.y;
				boundingBox_min.z = boundingBox_min.z >= vec3.z ? vec3.z : boundingBox_min.z;
				//
				boundingBox_max.x = boundingBox_max.x <= vec3.x ? vec3.x : boundingBox_max.x;
				boundingBox_max.y = boundingBox_max.y <= vec3.y ? vec3.y : boundingBox_max.y;
				boundingBox_max.z = boundingBox_max.z <= vec3.z ? vec3.z : boundingBox_max.z;
				modelData->boundingBox_min = boundingBox_min;
				modelData->boundingBox_max = boundingBox_max;
			}
			//
			if (mesh->HasNormals()) {
				vec3.x = mesh->mNormals[i].x;
				vec3.y = mesh->mNormals[i].y;
				vec3.z = mesh->mNormals[i].z;
				vertex.nor.push_back(vec3);
			}
			if (mesh->HasTangentsAndBitangents())
			{
				vec3.x = mesh->mTangents[i].x;
				vec3.y = mesh->mTangents[i].y;
				vec3.z = mesh->mTangents[i].z;
				vertex.tan.push_back(vec3);
			}
			if (mesh->HasVertexColors(0))
			{
				vec4.x = mesh->mColors[0][i].r;
				vec4.y = mesh->mColors[0][i].g;
				vec4.z = mesh->mColors[0][i].b;
				vec4.w = mesh->mColors[0][i].a;
				vertex.col.push_back(vec4);
			}
			//UV 01
			vec4 = glm::vec4(0);
			if (mesh->HasTextureCoords(0))
			{
				vec4.x = mesh->mTextureCoords[0][i].x;
				vec4.y = mesh->mTextureCoords[0][i].y;
				modelData->uv_0_1_abs_max.x = max(modelData->uv_0_1_abs_max.x, glm::abs(vec4.x));
				modelData->uv_0_1_abs_max.y = max(modelData->uv_0_1_abs_max.y, glm::abs(vec4.y));
			}
			if (mesh->HasTextureCoords(1))
			{
				vec4.z = mesh->mTextureCoords[1][i].x;
				vec4.w = mesh->mTextureCoords[1][i].y;
				modelData->uv_0_1_abs_max.z = max(modelData->uv_0_1_abs_max.z, glm::abs(vec4.z));
				modelData->uv_0_1_abs_max.w = max(modelData->uv_0_1_abs_max.w, glm::abs(vec4.w));
			}
			vertex.uv01.push_back(vec4);
			//UV 23
			vec4 = glm::vec4(0);
			if (mesh->HasTextureCoords(2))
			{
				vec4.x = mesh->mTextureCoords[2][i].x;
				vec4.y = mesh->mTextureCoords[2][i].y;
				modelData->uv_2_3_abs_max.x = max(modelData->uv_2_3_abs_max.x, glm::abs(vec4.x));
				modelData->uv_2_3_abs_max.y = max(modelData->uv_2_3_abs_max.y, glm::abs(vec4.y));
			}
			if (mesh->HasTextureCoords(3))
			{
				vec4.z = mesh->mTextureCoords[3][i].x;
				vec4.w = mesh->mTextureCoords[3][i].y;
				modelData->uv_2_3_abs_max.z = max(modelData->uv_2_3_abs_max.z, glm::abs(vec4.z));
				modelData->uv_2_3_abs_max.w = max(modelData->uv_2_3_abs_max.w, glm::abs(vec4.w));
			}
			vertex.uv23.push_back(vec4);
		}
		newData.vertexData = vertex;
		//indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			if (mesh->mFaces[i].mNumIndices != 3)
			{
				MessageOut("Face indices number was not equal 3.", false, true, "255,255,0");
				modelData.reset();
				return NULL;
			}
			//
			newData.vertexData.vertexIndices.push_back(mesh->mFaces[i].mIndices[0]);
			newData.vertexData.vertexIndices.push_back(mesh->mFaces[i].mIndices[1]);
			newData.vertexData.vertexIndices.push_back(mesh->mFaces[i].mIndices[2]);
			newData.indexNum += 3;
		}

		//Materials
		uint32_t matNameSize = std::min((uint32_t)128, (uint32_t)strlen(scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str()));
		strcpy_s(newData.matName, matNameSize + 1, scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str());

		//Skin data
		if (mesh->HasBones())
		{

		}

		modelData->faces.push_back(newData);
	}
	//----------------------
	modelData->filePath = filePath;
	//_modelCache.emplace(std::make_pair(fbxPath, std::move(modelData)));

	dataPtr->SetData(std::move(modelData));

	return dataPtr->GetData();
}

bool ModelFileStream::BuildModelPrimitives(ModelData* data, std::vector<ModelPrimitive>& prims)
{
	if (data != NULL)
	{
		prims.resize(data->faces.size());
		for (int i = 0; i < data->faces.size(); i++)
		{
			prims[i].matSocketName = data->faces[i].matName;
			prims[i].boundingBox_min = data->boundingBox_min;
			prims[i].boundingBox_max = data->boundingBox_max;
			prims[i].vertexInput = data->faces[i].vertexData;
			prims[i].modelPrimitiveName = data->filePath;
		}
		return true;
	}
	return false;
}
