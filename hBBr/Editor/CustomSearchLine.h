#pragma once

#include <QWidget>
#include "ui_CustomSearchLine.h"
#include <functional>
#include <qpixmap.h>
class CustomSearchLine : public QWidget
{
	Q_OBJECT

public:
	CustomSearchLine(QWidget *parent = nullptr);
	~CustomSearchLine();

	Ui::CustomSearchLineClass ui;

	void SetLabel(QString text);

	void SetLabel(QPixmap pixmap);

	std::function<void(QLineEdit*)> _enterSearchCallback = [](QLineEdit* lineEdit) {};

protected:
	virtual void paintEvent(QPaintEvent* event);
};
