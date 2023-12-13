#pragma once

#include <QDialog>
#include "ui_LineEditDialog.h"
#include <functional>
class LineEditDialog : public QDialog
{
	Q_OBJECT

public:

	LineEditDialog(QString name, QWidget *parent = nullptr);
	~LineEditDialog();

	std::function<void()> EnterCallBack = []() {};
	std::function<void()> CancelCallBack = []() {};


	Ui::LineEditDialogClass ui;

private slots:

	void Enter();

	void Cancel();

};
