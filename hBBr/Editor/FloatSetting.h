#pragma once
#include <QWidget>
#include "qevent.h"
#include "qdesktopwidget.h"
#include "QSpinBox.h"
#include <QStyleOption.h>
#include <QPainter.h>
#include <qlineedit.h>
//void valueChanged(double);
//void textChanged(const QString&);
//void valueChanged(const QString&);

class FloatSetting : public QDoubleSpinBox
{
	Q_OBJECT
public:
	//Precision
	FloatSetting(QWidget *parent = Q_NULLPTR,float value = 0.0f, float step = 0.01f, int precision = 4);
	~FloatSetting();

public:
	void SetStep(float _step);
	void SetPrecision(int _precision);

	inline float GetStep()const { return this->singleStep(); }
	inline int GetPrecision()const { return this->decimals(); }
	inline float GetValue()const { return this->value(); }
	//inline QDoubleSpinBox* GetSpinBox()const { return the_x; }

	virtual QString textFromValue(double val) const
	{
		QRegExp rx;
		rx.setPattern("(\\.){0,1}0+$");
		return QString("%1").arg(val, 0, 'f', -1).replace(rx, "");
	}

private:
	//QDoubleSpinBox* the_x;

	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void paintEvent(QPaintEvent* event);
	void wheelEvent(QWheelEvent* event) override;

	bool bEnableMovingSetting = false;
	bool bSelectText = false;
	QPoint StartPos;
	int CurrentX;
	float AddStore;
	float CurrentValue;

};


