#pragma once

#include <QDialog>
#include "ui_DirtyAssetsManager.h"

class DirtyAssetsManager : public QDialog
{
	Q_OBJECT

public:
	DirtyAssetsManager(QWidget *parent = nullptr);
	~DirtyAssetsManager();

private:
	Ui::DirtyAssetsManagerClass ui;
};
