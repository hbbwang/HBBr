#pragma once

#include <QWidget>
#include "ui_CustomSearchLine.h"

class CustomSearchLine : public QWidget
{
	Q_OBJECT

public:
	CustomSearchLine(QWidget *parent = nullptr);
	~CustomSearchLine();

	Ui::CustomSearchLineClass ui;

};
