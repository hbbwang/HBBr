#pragma once
#include <memory>
#include "PassBase.h"
/* Imgui pass define */
class ImguiPass :public GraphicsPass
{
public:
	ImguiPass(class PassManager* manager) :GraphicsPass(manager) {}
	virtual ~ImguiPass();
	virtual void PassInit()override;
	virtual void PassUpdate()override;
	virtual void PassReset()override;
private:
	void ShowPerformance();

	struct ImGuiContext* _imguiContent = nullptr;

};
