#pragma once

#include <QWidget>
#include "ui_CollapsedWidget.h"

class CollapsedWidget : public QWidget
{
	Q_OBJECT

public:
	CollapsedWidget(bool isSubWidget,QWidget *parent = nullptr);
	~CollapsedWidget();
	Ui::CollapsedWidgetClass ui;

	void Collapse();
	void Expand();
	bool bIsCollapsed = false;

	bool _isSubWidget = false;

	int originHeight = 0;

protected:
	virtual void resizeEvent(QResizeEvent* event);
	virtual void paintEvent(QPaintEvent* event);

	void ResizeButton();


private slots:
	void TitleChanged(const QString& title);
	void CollapseOrExpand();
};
