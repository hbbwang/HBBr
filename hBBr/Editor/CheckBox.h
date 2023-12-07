#pragma once

#include <QWidget>
#include "ui_CheckBox.h"
#include "PropertyClass.h"
#include <functional>
#include "HString.h"

class CheckBox : public PropertyClass
{
	Q_OBJECT

public:
	CheckBox(QWidget* parent = Q_NULLPTR, bool bEnbale = false);
	CheckBox(HString name, QWidget* parent = Q_NULLPTR, bool bEnbale = false);
	~CheckBox();

	bool* _boolBind = nullptr;
	std::vector<bool>* _boolArrayBind = nullptr;
	int _boolArrayBindIndex = 0;
	std::function<void(bool)> _callback = [](bool bEnable) {};
	void SetAlignmentLeft();

	Ui::CheckBox ui;
private slots:
	void stateChanged(int state);
};
