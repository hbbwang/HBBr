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

	std::vector<std::function<void()>> _finishExec;
	QList<class DurtyAssetItem*> _allItems;
	DurtyAssetItem* _headerItem = nullptr;
	virtual void paintEvent(QPaintEvent* event)override;
private:
	Ui::DirtyAssetsManagerClass ui;

};
