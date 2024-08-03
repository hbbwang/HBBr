#pragma once
#include <memory>
#include "PassBase.h"
#include "functional"
#include <string>
/* Imgui pass define */
class ImguiPassEditor :public GraphicsPass
{
public:
	ImguiPassEditor(class VulkanRenderer* renderer);
	virtual ~ImguiPassEditor();
	virtual void PassInit()override;
	virtual void PassReset()override;
	void EndFrame();
	void PassUpdate(std::shared_ptr<Texture2D> finalColor);
	HBBR_API void AddGui(std::function<void(ImguiPassEditor*)> fun);
	HBBR_API VkDescriptorSet GetRenderView()const;
protected:
	void ShowPerformance();
private:
	struct ImGuiContext* _imguiContent = nullptr;
	std::vector<std::function<void(ImguiPassEditor*)>> _gui_extensions;
	std::shared_ptr<Texture2D> _renderViewTexture;
	std::shared_ptr<Texture2D> _renderViewTexture_old;
	std::shared_ptr<DescriptorSet> _renderView;
	uint32_t _currentImguiFrameIndex = 0;
};
