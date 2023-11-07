#pragma once

#include <QWidget>
#include <qlayout.h>
#include "ui_Inspector.h"
#include <qtimer.h>
class Inspector : public QWidget
{
	Q_OBJECT

public:
	Inspector(QWidget *parent = nullptr);
	
	~Inspector();

	QVBoxLayout* _layoutMain = NULL;

	QTimer* _updateTimer = NULL;

	static Inspector* _currentInspector;

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
	QWidget* mainWidget = NULL;
private slots:
	void TimerUpdate();
};
