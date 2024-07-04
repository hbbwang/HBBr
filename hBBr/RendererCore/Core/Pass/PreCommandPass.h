#pragma once

#include <memory>
#include "PassBase.h"

class PreCommandPass :public CommandPass
{
public:
	PreCommandPass(class PassManager* manager) :CommandPass(manager) {}
	virtual ~PreCommandPass();
	virtual void PassInit()override;
	virtual void PassUpdate()override;
};
