#pragma once
#include <QWidget>
#include "VulkanRenderer.h"
#include <qtimer.h>
class RenderView  : public QWidget
{
	Q_OBJECT

public:

	VulkanRenderer *_vkRenderer = NULL;

	RenderView(QWidget *parent);

	~RenderView();
private:
	QTimer* _renderTimer = NULL;

private slots:
	void FuncRender();

};
