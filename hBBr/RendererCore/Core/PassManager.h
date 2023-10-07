#pragma once
//PassManager类，管理所有渲染或者计算Pass
#include "Common.h"
#include "Texture.h"
#include <vector>
#include <map>
class VulkanRenderer;
class PassBase;

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

	HBBR_INLINE std::shared_ptr <SceneTexture> GetSceneTexture()const {
		return _sceneTextures;
	}

	HBBR_INLINE std::vector<std::shared_ptr<PassBase>> GetExecutePasses()const {
		return _executePasses;
	}

	HBBR_INLINE std::vector<VkFence> GetExecuteFences()const {
		return _executeFence;
	}

	/* Pass添加,passName必须唯一! */
	void AddPass(std::shared_ptr<PassBase> newPass, const char* passName);
private:

	class VulkanRenderer* _renderer;

	std::shared_ptr <SceneTexture> _sceneTextures;

	std::vector<std::shared_ptr<PassBase>> _passes;

	std::vector<std::shared_ptr<PassBase>> _executePasses;

	std::vector<VkFence> _executeFence;
};
