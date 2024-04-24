#pragma once

#include <QDialog>
#include <functional>
#include "ui_DirtyAssetsManager.h"

class DirtyAssetsManager : public QDialog
{
	Q_OBJECT
public:
	DirtyAssetsManager(QWidget *parent = nullptr);
	~DirtyAssetsManager();

	std::function<void()> _finishExec = []() {};

private:
	Ui::DirtyAssetsManagerClass ui;
};
