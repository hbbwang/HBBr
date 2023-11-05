#pragma once

#include <QWidget>
#include <qlayout.h>
#include "ui_Inspector.h"

class Inspector : public QWidget
{
	Q_OBJECT

public:
	Inspector(QWidget *parent = nullptr);
	
	~Inspector();

	QVBoxLayout* _layoutMain = NULL;

	void ClearInspector();

	void LoadInspector_Empty();

	void LoadInspector_GameObject(class GameObject* gameObj);

protected:

	virtual void closeEvent(QCloseEvent* event)override;

private:
	Ui::InspectorClass ui;
};
