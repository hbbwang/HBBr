#pragma once
#include <qopenglwidget.h>
#include "VulkanRenderer.h"
#include <qtimer.h>

class RenderView  : public QWidget
{
	Q_OBJECT

public:

	class VulkanRenderer* _vkRenderer = NULL;

	RenderView(QWidget *parent);

	~RenderView();

	QTimer* _renderTimer;
protected:
	//不使用Qt默认的绘制引擎
	//virtual QPaintEngine* paintEngine() const { return 0; }

	virtual void showEvent(QShowEvent* event)override;

	virtual void resizeEvent(QResizeEvent* event)override;

	virtual void closeEvent(QCloseEvent* event)override;

	virtual void paintEvent(QPaintEvent* event);

private slots:

	void UpdateRender();

};
