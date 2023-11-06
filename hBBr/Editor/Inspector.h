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

	void RefreshInspector();

	void ClearInspector();

	void LoadInspector_Empty();

	void LoadInspector_GameObject(std::weak_ptr<class GameObject> gameObj);

protected:

	virtual void closeEvent(QCloseEvent* event)override;

	std::weak_ptr<GameObject> _currentGameObject;

private:
	Ui::InspectorClass ui;
};
