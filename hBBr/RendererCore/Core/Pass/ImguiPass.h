#pragma once
#include <memory>
#include "PassBase.h"
#include "functional"
/* Imgui pass define */
class ImguiPass :public GraphicsPass
{
public:
	ImguiPass(class VulkanRenderer* renderer);
	virtual ~ImguiPass();
	virtual void PassInit()override;
	virtual void PassReset()override;
	void PassUpdate(std::vector<VkImageView> frameBuffers);
	HBBR_API void AddGui(std::function<void(struct ImGuiContext*)> fun);
protected:
	virtual void PassUpdate()override;
private:
	void ShowPerformance();
	struct ImGuiContext* _imguiContent = nullptr;
	std::vector<std::function<void(struct ImGuiContext*)>> _gui_extensions;
};
