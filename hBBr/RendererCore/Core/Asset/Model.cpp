#include "Model.h"

#include "ConsoleDebug.h"
#include "FileSystem.h"

#ifdef __ANDROID__
#include <assimp/include/assimp/port/AndroidJNI/AndroidJNIIOSystem.h>
#else
#ifdef NDEBUG
#pragma comment(lib,"assimp/lib/Static/Release/assimp-vc142-mt.lib")
#pragma comment(lib,"assimp/lib/Static/Release/zlibstatic.lib") 
#else
#pragma comment(lib,"assimp/lib/Debug/assimp-vc142-mt.lib") 
#endif
#endif

#include <assimp/include/assimp/Importer.hpp>      // C++ importer interface
#include <assimp/include/assimp/scene.h>			// C++ importer interface
#include <assimp/include/assimp/postprocess.h>     // Post processing flags

#include "ContentManager.h"

Model::~Model()
{
	for (auto& i : faces)
	{
		delete i;
		i = nullptr;
	}
}

std::shared_ptr<Model> Model::LoadAsset(HGUID guid)
{
	const auto modelAssets = ContentManager::Get()->GetAssets(AssetType::Model);
	HString guidStr = GUIDToString(guid);
	//从内容管理器查找资产
	auto it = modelAssets.find(guid);
	{	
		if (it == modelAssets.end())
		{
			MessageOut(HString("Can not find [" + guidStr + "] model in content manager."), false, false, "255,255,0");
			return nullptr;
		}
	}
	auto dataPtr = std::static_pointer_cast<AssetInfo<Model>>(it->second);

	//是否需要重新加载
	bool bReload = false;
	if (dataPtr->IsAssetLoad())
	{
		return dataPtr->GetData();
	}
	else if (!dataPtr->IsAssetLoad() && dataPtr->GetSharedPtr())
	{
		bReload = true;
	}

	//获取实际路径
	HString filePath = it->second->absFilePath;
	if (!FileSystem::FileExist(filePath.c_str()))
	{
		return nullptr;
	}
	//导入fbx文件
#if _DEBUG
	ConsoleDebug::print_endl("Import fbx model :" + filePath, "255,255,255");
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
		return nullptr;
	}
	if (!scene->HasMaterials())
	{
		MessageOut("Error,cannot find materials in this fbx file.Import failed.", false, true, "255,255,0");
		return nullptr;
	}
	if (!scene->HasMeshes())
	{
		MessageOut("Error,cannot find meshes in this fbx file.Import failed.", false, true, "255,255,0");
		return nullptr;
	}

	std::shared_ptr<Model> model;
	if (!bReload)
	{
		model.reset(new Model);
	}
	else
	{
		//重新刷新asset
		model = dataPtr->GetSharedPtr();
	}

	model->_assetInfo = dataPtr;
	glm::vec3 boundingBox_min = glm::vec3(0, 0, 0);
	glm::vec3 boundingBox_max = glm::vec3(0, 0, 0);
	//FBX会把不同材质分为多个Meshes,我们以此作为"面"
	model->faces.reserve(scene->mNumMeshes);
	for (unsigned int nm = 0; nm < scene->mNumMeshes; nm++)
	{
		aiMesh* mesh = scene->mMeshes[nm];
#if IS_EDITOR
		ConsoleDebug::print_endl("Found meshes successful.\nThere are " + HString::FromUInt(mesh->mNumVertices) + " vertices have been found.", "160,160,160");
#endif
		//
		FaceData* newData = new FaceData;
		newData->vertexNum = mesh->mNumVertices;
		model->faceNum += 1;
		//
		newData->vertexData.pos.reserve(mesh->mNumVertices);
		newData->vertexData.nor.reserve(mesh->mNumVertices);
		newData->vertexData.tan.reserve(mesh->mNumVertices);
		newData->vertexData.col.reserve(mesh->mNumVertices);
		newData->vertexData.uv01.reserve(mesh->mNumVertices);
		newData->vertexData.uv23.reserve(mesh->mNumVertices);
		newData->vertexData.uv45.reserve(mesh->mNumVertices);
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			glm::vec3 vec3 = glm::vec3(0);
			glm::vec4 vec4 = glm::vec4(0);
			vec3.x = mesh->mVertices[i].x;
			vec3.y = mesh->mVertices[i].y;
			vec3.z = mesh->mVertices[i].z;
			newData->vertexData.pos.push_back(vec3);
			{//计算Bounding Box大小
				boundingBox_min.x = boundingBox_min.x >= vec3.x ? vec3.x : boundingBox_min.x;
				boundingBox_min.y = boundingBox_min.y >= vec3.y ? vec3.y : boundingBox_min.y;
				boundingBox_min.z = boundingBox_min.z >= vec3.z ? vec3.z : boundingBox_min.z;
				//
				boundingBox_max.x = boundingBox_max.x <= vec3.x ? vec3.x : boundingBox_max.x;
				boundingBox_max.y = boundingBox_max.y <= vec3.y ? vec3.y : boundingBox_max.y;
				boundingBox_max.z = boundingBox_max.z <= vec3.z ? vec3.z : boundingBox_max.z;
				model->boundingBox_min = boundingBox_min;
				model->boundingBox_max = boundingBox_max;
			}
			//
			if (mesh->HasNormals()) {
				vec3.x = mesh->mNormals[i].x;
				vec3.y = mesh->mNormals[i].y;
				vec3.z = mesh->mNormals[i].z;
				newData->vertexData.nor.push_back(vec3);
			}
			if (mesh->HasTangentsAndBitangents())
			{
				vec3.x = mesh->mTangents[i].x;
				vec3.y = mesh->mTangents[i].y;
				vec3.z = mesh->mTangents[i].z;
				newData->vertexData.tan.push_back(vec3);
			}
			if (mesh->HasVertexColors(0))
			{
				vec4.x = mesh->mColors[0][i].r;
				vec4.y = mesh->mColors[0][i].g;
				vec4.z = mesh->mColors[0][i].b;
				vec4.w = mesh->mColors[0][i].a;
				newData->vertexData.col.push_back(vec4);
			}
			//UV 01
			vec4 = glm::vec4(0);
			if (mesh->HasTextureCoords(0))
			{
				vec4.x = mesh->mTextureCoords[0][i].x;
				vec4.y = mesh->mTextureCoords[0][i].y;
				model->uv_0_1_abs_max.x = glm::fmax(model->uv_0_1_abs_max.x, glm::abs(vec4.x));
				model->uv_0_1_abs_max.y = glm::fmax(model->uv_0_1_abs_max.y, glm::abs(vec4.y));
			}
			if (mesh->HasTextureCoords(1))
			{
				vec4.z = mesh->mTextureCoords[1][i].x;
				vec4.w = mesh->mTextureCoords[1][i].y;
				model->uv_0_1_abs_max.z = glm::fmax(model->uv_0_1_abs_max.z, glm::abs(vec4.z));
				model->uv_0_1_abs_max.w = glm::fmax(model->uv_0_1_abs_max.w, glm::abs(vec4.w));
			}
			newData->vertexData.uv01.push_back(vec4);
			//UV 23
			vec4 = glm::vec4(0);
			if (mesh->HasTextureCoords(2))
			{
				vec4.x = mesh->mTextureCoords[2][i].x;
				vec4.y = mesh->mTextureCoords[2][i].y;
				model->uv_2_3_abs_max.x = glm::fmax(model->uv_2_3_abs_max.x, glm::abs(vec4.x));
				model->uv_2_3_abs_max.y = glm::fmax(model->uv_2_3_abs_max.y, glm::abs(vec4.y));
			}
			if (mesh->HasTextureCoords(3))
			{
				vec4.z = mesh->mTextureCoords[3][i].x;
				vec4.w = mesh->mTextureCoords[3][i].y;
				model->uv_2_3_abs_max.z = glm::fmax(model->uv_2_3_abs_max.z, glm::abs(vec4.z));
				model->uv_2_3_abs_max.w = glm::fmax(model->uv_2_3_abs_max.w, glm::abs(vec4.w));
			}
			newData->vertexData.uv23.push_back(vec4);
			//UV 45
			vec4 = glm::vec4(0);
			if (mesh->HasTextureCoords(4))
			{
				vec4.x = mesh->mTextureCoords[4][i].x;
				vec4.y = mesh->mTextureCoords[4][i].y;
				model->uv_4_5_abs_max.x = glm::fmax(model->uv_4_5_abs_max.x, glm::abs(vec4.x));
				model->uv_4_5_abs_max.y = glm::fmax(model->uv_4_5_abs_max.y, glm::abs(vec4.y));
			}
			if (mesh->HasTextureCoords(5))
			{
				vec4.z = mesh->mTextureCoords[5][i].x;
				vec4.w = mesh->mTextureCoords[5][i].y;
				model->uv_4_5_abs_max.z = glm::fmax(model->uv_4_5_abs_max.z, glm::abs(vec4.z));
				model->uv_4_5_abs_max.w = glm::fmax(model->uv_4_5_abs_max.w, glm::abs(vec4.w));
			}
			newData->vertexData.uv45.push_back(vec4);
		}
		//indices
		//默认它是三角面，所以每个面3个Index
		newData->vertexData.vertexIndices.reserve(mesh->mNumFaces * 3);
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			if (mesh->mFaces[i].mNumIndices != 3)
			{
				MessageOut("Face indices number was not equal 3.", false, true, "255,255,0");
				model.reset();
				return nullptr;
			}
			//
			newData->vertexData.vertexIndices.push_back(mesh->mFaces[i].mIndices[0]);
			newData->vertexData.vertexIndices.push_back(mesh->mFaces[i].mIndices[1]);
			newData->vertexData.vertexIndices.push_back(mesh->mFaces[i].mIndices[2]);
			newData->indexNum += 3;
		}

		//Materials
		uint32_t matNameSize = std::min((uint32_t)128, (uint32_t)strlen(scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str()));
		strcpy_s(newData->matName, matNameSize + 1, scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str());

		//Skin data
		if (mesh->HasBones())
		{

		}
		model->faces.push_back(newData);
	}
	//----------------------
	//_modelCache.emplace(std::make_pair(fbxPath, std::move(Model)));
	dataPtr->SetData(model);
	return dataPtr->GetData();
}

bool Model::BuildModelPrimitives(Model* data, std::vector<ModelPrimitive*>& prims, class VulkanRenderer* renderer)
{
	if (data != nullptr)
	{
		prims.resize(data->faces.size());
		for (int i = 0; i < data->faces.size(); i++)
		{
			prims[i] = new ModelPrimitive();
			prims[i]->faceData = data->faces[i];
			prims[i]->boundingBox_min = data->boundingBox_min;
			prims[i]->boundingBox_max = data->boundingBox_max;
			prims[i]->modelPrimitiveName = data->_assetInfo.lock()->displayName;
			prims[i]->renderer = renderer;
		}
		return true;
	}
	return false;
}
#if IS_EDITOR
void Model::SaveAsset(HString path)
{
}
#endif
