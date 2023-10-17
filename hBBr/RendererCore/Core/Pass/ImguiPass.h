#pragma once
#include <memory>
#include "PassBase.h"
/* Imgui pass define */
class ImguiScreenPass :public GraphicsPass
{
public:
	ImguiScreenPass(VulkanRenderer* renderer) :GraphicsPass(renderer) {}
	virtual ~ImguiScreenPass();
	virtual void PassInit()override;
	virtual void PassUpdate()override;
	virtual void PassReset()override;
private:
	void ShowPerformance();
};
