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

protected:
	virtual void showEvent(QShowEvent* event)override;

	virtual void resizeEvent(QResizeEvent* event)override;

private:

	QTimer* _renderTimer = NULL;

private slots:
	void FuncRender();

};
