#pragma once
#include <qwidget.h>
#include "FormMain.h"
class SDLWidget : public QWidget
{
	Q_OBJECT
		friend class RenderView;
private:
	SDLWidget(QWidget* parent);
public:
	SDLWidget(QWidget* parent, QString titleName);

	~SDLWidget();

	void RendererUpdate();

	HWND _hwnd = nullptr;

	class VulkanForm* _rendererForm = nullptr;

	static VulkanForm* _mainRenderer;
};