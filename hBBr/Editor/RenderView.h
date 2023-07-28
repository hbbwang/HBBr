#pragma once
#include <QWidget>
#include "VulkanRenderer.h"
#include <qtimer.h>

class RenderView  : public QWidget
{
	Q_OBJECT

public:

	class VulkanRenderer* _vkRenderer = NULL;

	RenderView(QWidget *parent);

	~RenderView();


protected:

	virtual void showEvent(QShowEvent* event)override;

	virtual void resizeEvent(QResizeEvent* event)override;

	virtual void closeEvent(QCloseEvent* event)override;

	virtual void focusInEvent(QFocusEvent* event)override;

	virtual void focusOutEvent(QFocusEvent* event)override;

	virtual void mousePressEvent(QMouseEvent* event)override;

	virtual void mouseReleaseEvent(QMouseEvent* event)override;

private:

	QTimer* _renderTimer = NULL;

private slots:
	void FuncRender();

};
