#pragma once
//PassManager类，管理所有渲染或者计算Pass
#include "Common.h"
#include "SceneTexture.h"
#include "Texture2D.h"
#include <vector>
#include <map>
#include <Pass/PassType.h>
#include "Component/DirectionalLightComponent.h"

class VulkanRenderer;
class PassBase;

#define MaxLightingNum  64

class PassManager
{
	friend class VulkanRenderer;
	friend class DirectionalLightComponent;
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

	HBBR_INLINE LightingUniformBuffer* GetLightingUniformBuffer(){
		return &_lightUniformBuffer;
	}

	HBBR_INLINE void BindLightingParameter(DirectionalLightComponent* lightComp) {
		_lightings.push_back(lightComp);
	}

	HBBR_INLINE void UnBindLightingParameter(DirectionalLightComponent* lightComp) {
		auto it = std::remove(_lightings.begin(), _lightings.end(), lightComp);
		if (it != _lightings.end())
		{
			_lightings.erase(it);
		}
	}

	/* Pass添加,passName必须唯一! */
	void AddPass(std::shared_ptr<PassBase> newPass, const char* passName);

	void CmdCopyFinalColorToSwapchain();

	class VulkanRenderer* _renderer;

	void SetupPassUniformBuffer(class CameraComponent* camera , VkExtent2D renderSize);

	static glm::mat4 GetPerspectiveProjectionMatrix(float FOV, float w, float h, float nearPlane = 0.001, float farPlane = 100.0f, VkSurfaceTransformFlagBitsKHR surfaceTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);

	PassUniformBuffer GetPassUniformBufferCache()const { return _passUniformBuffer; }

private:

	std::shared_ptr <SceneTexture> _sceneTextures;

	std::vector<std::shared_ptr<PassBase>> _passes;

	std::vector<std::shared_ptr<PassBase>> _executePasses;

	//Pass Uniform
	PassUniformBuffer _passUniformBuffer;

	LightingUniformBuffer _lightUniformBuffer;

	std::vector<DirectionalLightComponent*> _lightings;

};
