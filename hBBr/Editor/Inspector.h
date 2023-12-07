#pragma once

#include <QWidget>
#include <qlayout.h>
#include "ui_Inspector.h"
#include <qtimer.h>
#include "qscrollarea.h"

class Inspector : public QWidget
{
	Q_OBJECT

public:
	Inspector(QWidget *parent = nullptr);
	
	~Inspector();

	//添加需要一同进行渲染主循环的PropertyClass
	QList<class PropertyClass*> _property_needUpdate;

	QVBoxLayout* _layoutMain = nullptr;

	QTimer* _updateTimer = nullptr;

	static Inspector* _currentInspector;

	void PropertyUpdate();

	void RefreshInspector();

	void ClearInspector();

	void LoadInspector_GameObject(std::weak_ptr<class GameObject> gameObj , bool bFoucsUpdate = false);

protected:

	virtual void closeEvent(QCloseEvent* event)override;

	virtual void resizeEvent(QResizeEvent* event)override;

	virtual void focusInEvent(QFocusEvent* event)override;

	virtual void focusOutEvent(QFocusEvent* event)override;

	std::weak_ptr<GameObject> _currentGameObject;

private:

	Ui::InspectorClass ui;
	QScrollArea* scrollArea = nullptr;
	QWidget* scrollWidget = nullptr;

private slots:
	void TimerUpdate();
};
