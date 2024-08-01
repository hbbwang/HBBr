#pragma once

#include <QMainWindow>
#include <qtablewidget.h>
#include <qtreewidget.h>
#include <ContentManager.h>
#include "ui_PerformanceWatcher.h"
#include <qtimer.h>
#include <map>
#include <vector>

struct AssetItem
{
	std::weak_ptr<class AssetObject> object;
	QTreeWidgetItem* item = nullptr;
};

struct RenderingTileItem
{
	QTreeWidgetItem* item_parent = nullptr;
	QTreeWidgetItem* item_cpu = nullptr;
	QTreeWidgetItem* item_gpu = nullptr;
	class PassBase* pass = nullptr;
};

class PerformanceWatcher : public QMainWindow
{
	Q_OBJECT

public:
	PerformanceWatcher(QWidget *parent = nullptr);
	~PerformanceWatcher();

	QTabWidget*				_tabWidget = nullptr;
	
	//-----AssetWatcher 
	
	class QTreeWidget*		_assetWatcherTree = nullptr;

	std::map<class AssetInfoBase*, AssetItem> _assets;

	//-----RenderingTimeWatcher

	class QTreeWidget*						_renderingTimeWatcherTree = nullptr;

	std::vector<RenderingTileItem>	_renderingTimes;
	RenderingTileItem	_frame;
	//-----

	QTimer*					_timer = nullptr;

	QLabel*					_gcStaus = nullptr;



protected:
	virtual void closeEvent(QCloseEvent* event);

	void UpdateAssetWatcher();

	void UpdateRenderingTimeWatcher();

	QTreeWidgetItem* AddTreeTopItem(QTreeWidget* tree, QString itemName);

private:
	Ui::PerformanceWatcherClass ui;
};
