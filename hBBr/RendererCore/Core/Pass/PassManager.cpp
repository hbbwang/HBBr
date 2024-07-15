#include "PassManager.h"
#include "VulkanRenderer.h"
#include "PassBase.h"
#include "Pass/BasePass.h"
#include "Pass/DeferredLightingPass.h"
#include "Pass/ImguiPass.h"
#include "Pass/GUIPass.h"
#include "Pass/PreCommandPass.h"
#include "Pass/PostProcessPass.h"
#include "Component/CameraComponent.h"
PassManager::PassManager(VulkanRenderer* renderer)
{
	_renderer = renderer;
	_lightings.reserve(MaxLightingNum);
	_lightUniformBuffer = {};
	_sceneTextures.reset(new SceneTexture(renderer));
	{
		//Precommand Pass
		std::shared_ptr<PreCommandPass> precommand = std::make_shared<PreCommandPass>(this);
		AddPass(precommand, "PreCommand");
		//Opaque Pass
		std::shared_ptr<BasePass> basePass = std::make_shared<BasePass>(this);
		AddPass(basePass, "Base Pass");
		//Deferred Lighting Pass
		std::shared_ptr<DeferredLightingPass> deferredLighting = std::make_shared<DeferredLightingPass>(this);
		AddPass(deferredLighting, "Deferred Lighting");
		//Post Process Pass
		std::shared_ptr<PostProcessPass> postProcess = std::make_shared<PostProcessPass>(this);
		AddPass(postProcess, "Post Process");
		//Screen GUI Pass
		std::shared_ptr<GUIPass> gui = std::make_shared<GUIPass>(this);
		AddPass(gui, "GUI");
		//#ifdef IS_EDITOR
		//		std::shared_ptr<ImguiScreenPass> imgui = std::make_shared<ImguiScreenPass>(renderer);
		//		AddPass(imgui, "Imgui");
		//#endif
	}
	for (auto p : _passes)
	{
		p->PassInit();
	}
}

void PassManager::PassesUpdate()
{
	const uint32_t frameIndex =_renderer->_currentFrameIndex;

	_sceneTextures->UpdateTextures();

	//Update lighting uniform buffer
	_lightUniformBuffer.validLightCount = (uint32_t)_lightings.size();
	int lightIndex = 0;
	for (auto& i : _lightings)
	{
		_lightUniformBuffer.passUniform = _passUniformBuffer;
		_lightUniformBuffer.lightParams[lightIndex].LightColor = i->GetLightColor();
		_lightUniformBuffer.lightParams[lightIndex].LightStrength = i->GetLightIntensity();
		_lightUniformBuffer.lightParams[lightIndex].LightDirection = -(i->GetTransform()->GetForwardVector());
		_lightUniformBuffer.lightParams[lightIndex].LightPosition = i->GetTransform()->GetWorldLocation();
		_lightUniformBuffer.lightParams[lightIndex].LightType = i->GetLightType();
		_lightUniformBuffer.lightParams[lightIndex].LightSpecular = i->GetLightSpecular();
		//Flags
		if (i->IsCastShadow())
		{
			_lightUniformBuffer.lightParams[lightIndex].LightFlags |= LightingFlagBits::LightingFlag_CastShadow;
		}
		lightIndex++;
	}

	//Update post process uniform buffer
	_postProcessUniformBuffer.passUniform = _passUniformBuffer;
	_postProcessUniformBuffer.debugMode = 0;

	//Collect render setting (Commandbuffer record)
	_executePasses.clear();
	for (auto p : _passes)
	{
		p->PassUpdate();
		_executePasses.push_back(p);
	}
}

void PassManager::PassesRelease()
{
	_passes.clear();
	_sceneTextures.reset();
}

void PassManager::PassesReset()
{
	if (_sceneTextures == nullptr)
		return;
	_sceneTextures->UpdateTextures();
	for (auto& p : _passes)
	{
		p->Reset();
	}
}

void PassManager::AddPass(std::shared_ptr<PassBase> newPass, const char* passName)
{
	if (newPass == nullptr)
	{
		MessageOut("Add Pass Failed.The New Pass Is Null.", true, true);
	}
	auto it = std::find_if(_passes.begin(), _passes.end(), [newPass](std::shared_ptr<PassBase> &inPass) {
		return inPass->GetName() == newPass->GetName();
	});
	if (it != _passes.end())
	{
		MessageOut("Add Pass Failed.Pass Name Has Been Exist.", true, true);
	}
	newPass->_passName = passName;
	_passes.push_back(newPass);
}

void PassManager::CmdCopyFinalColorToSwapchain()
{
	const auto& manager = VulkanManager::GetManager();
	const auto cmdBuf = _renderer->GetCommandBuffer();
	int swapchainIndex = _renderer->GetCurrentFrameIndex();
	auto swapchainImage = _renderer->_swapchainImages[swapchainIndex];
	auto finalRT = this->GetSceneTexture()->GetTexture(SceneTextureDesc::FinalColor);
	manager->Transition(cmdBuf, swapchainImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	finalRT->Transition(cmdBuf, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	manager->CmdColorBitImage(cmdBuf, finalRT->GetTexture(), swapchainImage, _renderer->GetRenderSize(), _renderer->GetWindowSurfaceSize());
	finalRT->Transition(cmdBuf, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	manager->Transition(cmdBuf, swapchainImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

void PassManager::SetupPassUniformBuffer(CameraComponent* camera, VkExtent2D renderSize)
{
	if (!_renderer->IsWorldValid())
	{
		return;
	}
	if (camera != nullptr)
	{
		_passUniformBuffer.View = camera->GetViewMatrix();
		_passUniformBuffer.View_Inv = camera->GetInvViewMatrix();

		_passUniformBuffer.Projection = GetPerspectiveProjectionMatrix(
			camera->GetFOV(), 
			(float)renderSize.width, 
			(float)renderSize.height,
			camera->GetNearClipPlane(), 
			camera->GetFarClipPlane(), 
			_renderer->_surfaceCapabilities.currentTransform
		);

		_passUniformBuffer.Projection_Inv = glm::inverse(_passUniformBuffer.Projection);
		_passUniformBuffer.ViewProj = _passUniformBuffer.Projection * _passUniformBuffer.View;

		_passUniformBuffer.ViewProj_Inv = glm::inverse(_passUniformBuffer.ViewProj);
		_passUniformBuffer.ScreenInfo = glm::vec4((float)renderSize.width, (float)renderSize.height, camera->GetNearClipPlane(), camera->GetFarClipPlane());
		auto trans = camera->GetGameObject()->GetTransform();
		_passUniformBuffer.CameraPos_GameTime = glm::vec4(trans->GetWorldLocation().x, trans->GetWorldLocation().y, trans->GetWorldLocation().z, (float)VulkanApp::GetGameTime());
		auto viewDir = glm::normalize(trans->GetForwardVector());
		_passUniformBuffer.CameraDirection = glm::vec4(viewDir.x, viewDir.y, viewDir.z, 0.0f);

	}
}

glm::mat4 PassManager::GetPerspectiveProjectionMatrix(float FOV, float w, float h, float nearPlane, float  farPlane, VkSurfaceTransformFlagBitsKHR surfaceTransform)
{		
	//DirectX Left hand 
	glm::mat4 flipYMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 0.5f));
	glm::mat4 pre_rotate_mat = glm::mat4(1);
	float aspect = (float)w / (float)h;
	glm::vec3 rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
	if (surfaceTransform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) {
		pre_rotate_mat = glm::rotate(pre_rotate_mat, glm::radians(90.0f), rotation_axis);
		aspect = (float)h / (float)w;
	}
	else if (surfaceTransform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR) {
		pre_rotate_mat = glm::rotate(pre_rotate_mat, glm::radians(270.0f), rotation_axis);
		aspect = (float)h / (float)w;
	}
	else if (surfaceTransform & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR) {
		pre_rotate_mat = glm::rotate(pre_rotate_mat, glm::radians(180.0f), rotation_axis);
	}
	//DirectX Left hand.
	return pre_rotate_mat * flipYMatrix * glm::perspectiveLH(glm::radians(FOV), aspect, nearPlane, farPlane);
}
