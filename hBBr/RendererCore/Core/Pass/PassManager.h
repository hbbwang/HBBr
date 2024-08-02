#pragma once
//PassManager类，管理所有渲染或者计算Pass
#include "Common.h"
#include "SceneTexture.h"
#include "Texture2D.h"
#include <vector>
#include <map>
#include <Pass/PassType.h>
#include <Pass/UniformBuffer.h>
#include "Component/DirectionalLightComponent.h"

class VulkanRenderer;
class PassBase;

struct InsertPassItem
{
	std::shared_ptr<PassBase> pass = nullptr;
	uint32_t index = 0;
};

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

	HBBR_API HBBR_INLINE std::shared_ptr <SceneTexture> GetSceneTexture()const {
		return _sceneTextures;
	}

	HBBR_API HBBR_INLINE std::vector<std::shared_ptr<PassBase>> GetExecutePasses()const {
		return _executePasses;
	}

	HBBR_API HBBR_INLINE std::vector<std::shared_ptr<PassBase>> GetInitPasses()const {
		return _passes;
	}

	HBBR_API HBBR_INLINE LightingUniformBuffer* GetLightingUniformBuffer(){
		return &_lightUniformBuffer;
	}

	HBBR_API HBBR_INLINE PostProcessUniformBuffer* GetPostProcessUniformBuffer() {
		return &_postProcessUniformBuffer;
	}

	HBBR_API HBBR_INLINE void BindLightingParameter(DirectionalLightComponent* lightComp) {
		_lightings.push_back(lightComp);
	}

	HBBR_API HBBR_INLINE void UnBindLightingParameter(DirectionalLightComponent* lightComp) {
		auto it = std::remove(_lightings.begin(), _lightings.end(), lightComp);
		if (it != _lightings.end())
		{
			_lightings.erase(it);
		}
	}

	/* 初始化Pass添加,passName最好唯一 */
	void AddPass(std::shared_ptr<PassBase> newPass, const char* passName);

	void CmdCopyFinalColorToSwapchain();

	class VulkanRenderer* _renderer;

	void SetupPassUniformBuffer(class CameraComponent* camera , VkExtent2D renderSize);

	HBBR_API static glm::mat4 GetPerspectiveProjectionMatrix(float FOV, float w, float h, float nearPlane = 0.001, float farPlane = 100.0f, VkSurfaceTransformFlagBitsKHR surfaceTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);

	HBBR_API PassUniformBuffer GetPassUniformBufferCache()const { return _passUniformBuffer; }

private:

	std::shared_ptr <SceneTexture> _sceneTextures;

	std::vector<std::shared_ptr<PassBase>> _passes;

	std::vector<std::shared_ptr<PassBase>> _executePasses;

	//Pass Uniform
	PassUniformBuffer _passUniformBuffer;

	LightingUniformBuffer _lightUniformBuffer;

	std::vector<DirectionalLightComponent*> _lightings;

	PostProcessUniformBuffer _postProcessUniformBuffer;

};
