#pragma once
//PassManager类，管理所有渲染或者计算Pass
#include "Common.h"
#include "Texture.h"
#include <vector>
#include <map>
class VulkanRenderer;
class PassBase;
enum class SceneTextureDesc{
	SceneColor = 0,
	DepthStencil = 1,
};

class SceneTexture
{
public:
	SceneTexture(VulkanRenderer* renderer);
	~SceneTexture() 
	{
		_sceneTexture.clear();
	}
	void UpdateTextures();
	std::shared_ptr<Texture> GetTexture(SceneTextureDesc desc)
	{
		auto it = _sceneTexture.find(desc);
		if (it != _sceneTexture.end())
		{
			return it->second;
		}
		return NULL;
	}
private:
	std::map<SceneTextureDesc, std::shared_ptr<Texture>> _sceneTexture;
};

class PassManager
{
	friend class VulkanRenderer;
public:
	~PassManager() 
	{
		PassesRelease();
	}
	void PassesInit(VulkanRenderer* renderer);
	void PassesUpdate();
	void PassesRelease();
	void PassesReset();
	__forceinline std::shared_ptr <SceneTexture> GetSceneTexture()const {
		return _sceneTextures;
	}

	/* Pass添加,passName必须唯一! */
	void AddPass(std::shared_ptr<PassBase> newPass, const char* passName);
private:

	class VulkanRenderer* _renderer;

	std::map<HString, std::shared_ptr<PassBase>> _passes;

	std::shared_ptr <SceneTexture> _sceneTextures;

	std::vector<VkFence> _executeFence;
};
