#pragma once

#include <QMainWindow>
#include <qtablewidget.h>
#include <qtreewidget.h>
#include <ContentManager.h>
#include "ui_MemoryWatcher.h"
#include <qtimer.h>
#include <map>
#include <vector>

struct AssetItem
{
	std::weak_ptr<class AssetObject> object;
	QTreeWidgetItem* item = nullptr;
};

class MemoryWatcher : public QMainWindow
{
	Q_OBJECT

public:
	MemoryWatcher(QWidget *parent = nullptr);
	~MemoryWatcher();

	QTabWidget*				_tabWidget = nullptr;
		
	QWidget*				_assetWatcher = nullptr;

	QTimer*					_timer = nullptr;

	QLabel*					_gcStaus = nullptr;

	std::map<class AssetInfoBase*, AssetItem> _assets;

	class QTreeWidget*		_assetWatcherTree = nullptr;

protected:
	virtual void closeEvent(QCloseEvent* event);
	void UpdateAssetWatcher();

private:
	Ui::MemoryWatcherClass ui;
};
