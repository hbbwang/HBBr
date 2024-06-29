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

	virtual int exec()override;

	std::vector<std::function<void()>> _finishExec;

	QList<class DurtyAssetItem*> _allItems;

	DurtyAssetItem* _headerItem = nullptr;
	virtual void paintEvent(QPaintEvent* event)override;
	virtual void closeEvent(QCloseEvent* e)override;
private:
	Ui::DirtyAssetsManagerClass ui;

};
