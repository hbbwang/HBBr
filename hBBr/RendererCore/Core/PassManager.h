﻿#pragma once
//PassManager类，管理所有渲染或者计算Pass
#include "Common.h"
#include "Texture2D.h"
#include <vector>
#include <map>
#include <Pass/PassType.h>
class VulkanRenderer;
class PassBase;

class PassManager
{
	friend class VulkanRenderer;
public:
	PassManager(VulkanRenderer* renderer);
	~PassManager() 
	{
		PassesRelease();
	}
	void PassesUpdate();
	void PassesRelease();
	void PassesReset();

	HBBR_INLINE std::shared_ptr <SceneTexture> GetSceneTexture()const {
		return _sceneTextures;
	}

	HBBR_INLINE std::vector<std::shared_ptr<PassBase>> GetExecutePasses()const {
		return _executePasses;
	}

	/* Pass添加,passName必须唯一! */
	void AddPass(std::shared_ptr<PassBase> newPass, const char* passName);

	void CmdCopyFinalColorToSwapchain();

	class VulkanRenderer* _renderer;

	void SetupPassUniformBuffer(class CameraComponent* camera , VkExtent2D renderSize);

	PassUniformBuffer GetPassUniformBufferCache()const { return _passUniformBuffer; }

private:

	std::shared_ptr <SceneTexture> _sceneTextures;

	std::vector<std::shared_ptr<PassBase>> _passes;

	std::vector<std::shared_ptr<PassBase>> _executePasses;

	//Pass Uniform
	PassUniformBuffer _passUniformBuffer;


};
