#pragma once
#include <QWidget>
#include <qtimer.h>
#include "FormMain.h"
#include "SDLWidget.h"
class RenderView  : public QWidget
{
	Q_OBJECT

public:

	RenderView(QWidget *parent);

	~RenderView();

	void Update();

	SDLWidget* _mainRendererWidget = nullptr;

	static SDL_Keycode mapQtKeyToSdlKey(int qtKey);

protected:
	//不使用Qt默认的绘制引擎
	//virtual QPaintEngine* paintEngine() const { return 0; }

	virtual void resizeEvent(QResizeEvent* event)override;

	virtual void closeEvent(QCloseEvent* event)override;

	virtual void paintEvent(QPaintEvent* event)override;

	virtual void dragEnterEvent(QDragEnterEvent* event)override;

	virtual void dropEvent(QDropEvent* event)override;

};
