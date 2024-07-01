#pragma once
#include <memory>
#include "PassBase.h"

class FinalPass :public CommandPass
{
public:
	FinalPass(class PassManager* manager) :CommandPass(manager) {}
	virtual ~FinalPass();
	virtual void PassInit()override;
	virtual void PassUpdate()override;
};
