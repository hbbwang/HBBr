#pragma once
#include "FormMain.h"

class EditorMain
{
public:
	EditorMain();

	VulkanForm* _mainForm = nullptr;

	class ImguiPassEditor* _editorGui = nullptr;

};

