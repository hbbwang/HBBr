#pragma once
#include <memory>
#include "PassBase.h"
#include "functional"
/* Imgui pass define */
class ImguiPassEditor :public GraphicsPass
{
public:
	ImguiPassEditor(class VulkanRenderer* renderer);
	virtual ~ImguiPassEditor();
	virtual void PassInit()override;
	virtual void PassReset()override;
	void PassUpdate(std::vector<VkImageView> frameBuffers);
	HBBR_API void AddGui(std::function<void(struct ImGuiContext*)> fun);
protected:
	virtual void PassUpdate()override;
private:
	struct ImGuiContext* _imguiContent = nullptr;
	std::vector<std::function<void(struct ImGuiContext*)>> _gui_extensions;
};
