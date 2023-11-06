#pragma once

#include <QWidget>
#include "ui_ArraySetting.h"
#include <QPaintEvent>
#include <qlist.h>
#include "PropertyClass.h"
#include"qlistwidget.h"

class ArraySetting : public QWidget
{
	Q_OBJECT

public:
	ArraySetting(QString Name,QWidget *parent = Q_NULLPTR, QString type = "float");
	~ArraySetting();
	Ui::ArraySetting ui;

	QList<PropertyClass*> arraylist;
	QList<QListWidgetItem*> arrayitem;
	int typeID=0;
	int lastArrayCount = 0;
	
	void AddNewVaule();
	void RemoveValue(int index);


protected:
	virtual void paintEvent(QPaintEvent* event);
private slots:
	virtual void valueChange(int num);
	virtual void deleteItem();
};
