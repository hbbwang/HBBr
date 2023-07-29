#pragma once
#include <qopenglwidget.h>
#include <qtimer.h>


class RenderView  : public QWidget
{
	Q_OBJECT

public:

	RenderView(QWidget *parent);

	~RenderView();

	QTimer* _renderTimer;

protected:
	//不使用Qt默认的绘制引擎
	//virtual QPaintEngine* paintEngine() const { return 0; }


	bool event(QEvent* event) override;

	virtual void focusInEvent(QFocusEvent* event)override;

	virtual void focusOutEvent(QFocusEvent* event)override;

	virtual void mousePressEvent(QMouseEvent* event)override;

	virtual void mouseReleaseEvent(QMouseEvent* event)override;

	virtual void showEvent(QShowEvent* event)override;

	virtual void resizeEvent(QResizeEvent* event)override;

	virtual void closeEvent(QCloseEvent* event)override;

	virtual void paintEvent(QPaintEvent* event);


private slots:

	void UpdateRender();

};
