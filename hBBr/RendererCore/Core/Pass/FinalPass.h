#pragma once
#include <memory>
#include "PassBase.h"

class FinalPass :public CommandPass
{
public:
	FinalPass(VulkanRenderer* renderer) :CommandPass(renderer) {}
	virtual ~FinalPass();
	virtual void PassInit()override;
	virtual void PassUpdate()override;
};
