#pragma once
#include <memory>
#include "PassBase.h"
#include "functional"
/* Imgui pass define */
class ImguiPass :public GraphicsPass
{
public:
	ImguiPass(class PassManager* manager);
	virtual ~ImguiPass();
	virtual void PassInit()override;
	virtual void PassReset()override;
	void EndFrame();
	HBBR_API void AddGui(std::function<void(struct ImGuiContext*)> fun);
	HBBR_API void EnableOutputRT(bool bEnable);
	HBBR_API VkDescriptorSet GetRenderView()const;
protected:
	virtual void PassUpdate()override;
private:
	void ShowPerformance();
	struct ImGuiContext* _imguiContent = nullptr;
	std::vector<std::function<void(struct ImGuiContext*)>> _gui_extensions;
	bool bSpawnOutputRT;
	std::shared_ptr<Texture2D> _renderViewTexture;
	std::shared_ptr<DescriptorSet> _renderView;
	class VulkanSwapchain* _swapchain = nullptr;
};
