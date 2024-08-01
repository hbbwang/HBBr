#pragma once
#include <memory>
#include "PassBase.h"
/* Imgui pass define */
class ImguiScreenPass :public GraphicsPass
{
public:
	ImguiScreenPass(class PassManager* manager) :GraphicsPass(manager) {}
	virtual ~ImguiScreenPass();
	virtual void PassInit()override;
	virtual void PassUpdate()override;
	virtual void PassReset()override;
private:
	void ShowPerformance();

	struct ImGuiContext* _imguiContent = nullptr;

};
