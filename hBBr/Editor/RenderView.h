#pragma once
#include <QWidget>
#include "VulkanRenderer.h"
class RenderView  : public QWidget
{
	Q_OBJECT

public:

	VulkanRenderer *_vkRenderer = NULL;

	RenderView(QWidget *parent);

	~RenderView();
};
