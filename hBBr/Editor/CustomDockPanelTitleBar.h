#pragma once

#include <QWidget>
#include "ui_CustomDockPanelTitleBar.h"

class CustomDockPanelTitleBar  : public QWidget
{
	Q_OBJECT

public:
	CustomDockPanelTitleBar(QWidget *parent);
	~CustomDockPanelTitleBar();

	QWidget* _parent = nullptr;

protected:
	virtual void paintEvent(QPaintEvent* event);
private:
	Ui::DockPanelTitleBar ui;
private slots:
	void TitleChange(const QString& newTitle);
};
