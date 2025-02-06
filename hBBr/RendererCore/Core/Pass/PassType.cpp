#include "PassType.h"

std::string GetPassName(Pass& pass)
{
	switch (pass)
	{
		case Pass::PreCommand:return "PreCommand";
		case Pass::BasePass:return "BasePass";
		case Pass::DeferredLighting:return "DeferredLighting";
		case Pass::PostProcess:return "PostProcess";
		case Pass::Imgui:return "Imgui";
		case Pass::MaxNum:
		default:return "UnknowPass";
	}
	return "UnknowPass";
}