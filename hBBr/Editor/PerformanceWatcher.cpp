#include "PerformanceWatcher.h"
#include "qlabel.h"
#include "FormMain.h"
#include "EditorCommonFunction.h"
#include "AssetObject.h"
#include "VulkanObjectManager.h"
#include "VulkanRenderer.h"
#include "Pass/PassManager.h"
#include "Pass/PassBase.h"
#include "HTime.h"
#if _WIN32
#include <Psapi.h>
void ProcessMemoryCounters(size_t& memoryUsage)
{
	DWORD process_id = GetCurrentProcessId();
	HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
	if (process_handle) {
		PROCESS_MEMORY_COUNTERS pmc;
		if (GetProcessMemoryInfo(process_handle, &pmc, sizeof(pmc))) {
			memoryUsage = pmc.WorkingSetSize;
		}
		CloseHandle(process_handle);
	}
}
#endif

PerformanceWatcher::PerformanceWatcher(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	_tabWidget = new QTabWidget(this);
	_gcStaus = new QLabel(this);
	ui.mainVerticalLayout->addWidget(_gcStaus);
	ui.mainVerticalLayout->addWidget(_tabWidget);
	ui.mainVerticalLayout->setStretch(0, 0);
	ui.mainVerticalLayout->setStretch(1, 0);
	ui.mainVerticalLayout->setStretch(2, 100);
	ui.MemoryStaus->setAlignment(Qt::AlignLeft);

	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlag(Qt::Window);

	_timer = new QTimer(this);
	_timer->setSingleShot(false);
	_timer->setInterval(500);//0.5秒更新一次
	connect(_timer, &QTimer::timeout, this, [this]() 
		{
			// 更新内存使用情况
			{
				size_t memoryUsage = 0;
				#if _WIN32
				ProcessMemoryCounters(memoryUsage);
				#endif
				QString text = QString::fromLocal8Bit("内存使用 : ") + QString::number(memoryUsage / 1024);
				text += QString(" kb(") + QString::number(memoryUsage / 1024 / 1024) + " mb)";
				ui.MemoryStaus->setText(text);
			}
			if (VulkanApp::GetForms().size() <= 0)
				return;
			// GC倒计时
			{
				QString text = QString::fromLocal8Bit("GC剩余倒计时 : ") + QString::number(int(VulkanObjectManager::Get()->GetMaxGCTime() - VulkanObjectManager::Get()->GetGCTime())); 
				_gcStaus->setText(text);
			}
			//
			if (_tabWidget->currentIndex() == 0)
			{
				if (_timer->interval() != 500)
					_timer->setInterval(500);
				UpdateAssetWatcher();
			}		
			else if (_tabWidget->currentIndex() == 1)
			{
				if (_timer->interval() != 100)
					_timer->setInterval(100);
				UpdateRenderingTimeWatcher();
			}
		});
	_timer->start();

	LoadEditorWindowSetting(this, "PerformanceWatcher");

	//Asset Watcher
	{
		_assetWatcherTree = new QTreeWidget(this);
		_tabWidget->addTab(_assetWatcherTree, GetEditorInternationalization("PerformanceWatcher", "AssetWatch-Ttile"));
		_assetWatcherTree->setHeaderLabels(
			{
				GetEditorInternationalization("PerformanceWatcher","AssetWatch-Header0"),
				GetEditorInternationalization("PerformanceWatcher","AssetWatch-Header1"),
				GetEditorInternationalization("PerformanceWatcher","AssetWatch-Header2"),
				GetEditorInternationalization("PerformanceWatcher","AssetWatch-Header3"),
			});
		for (int i = 1; i < (int)AssetType::MaxNum; i++)
		{
			QTreeWidgetItem* item = new QTreeWidgetItem(_assetWatcherTree);
			item->setText(0, GetAssetTypeString((AssetType)i).c_str());
			_assetWatcherTree->addTopLevelItem(item);
		}
		_assetWatcherTree->setColumnWidth(0, this->width() * 0.2f);
		_assetWatcherTree->setColumnWidth(1, this->width() * 0.1f);
		_assetWatcherTree->setColumnWidth(2, this->width() * 0.4f);
		_assetWatcherTree->setColumnWidth(3, this->width() * 0.3f);
	}

	//Rendering Time Watcher 渲染耗时(ms)
	{
		_renderingTimeWatcherTree = new QTreeWidget(this);
		_tabWidget->addTab(_renderingTimeWatcherTree, GetEditorInternationalization("PerformanceWatcher", "RenderingTimeWatch-Ttile"));
		_renderingTimeWatcherTree->setHeaderLabels(
			{
				GetEditorInternationalization("PerformanceWatcher","RenderingTimeWatch-Header0"),
				GetEditorInternationalization("PerformanceWatcher","RenderingTimeWatch-Header1"),
			});
		//一帧总时间
		{
			_frame.item_parent = AddTreeTopItem(_renderingTimeWatcherTree, "Frame");
			//CPU
			{
				_frame.item_cpu = new QTreeWidgetItem(_frame.item_parent);
				_frame.item_cpu->setText(0,"CPU");
			}
			//GPU
			{
				_frame.item_gpu = new QTreeWidgetItem(_frame.item_parent);
				_frame.item_gpu->setText(0, "GPU");
			}
		}
		//每个Pass
		{
			auto passes = AddTreeTopItem(_renderingTimeWatcherTree, "Passes");
			for (auto& i : VulkanApp::GetMainForm()->renderer->GetPassManagers())
			{
				for (auto& p : i.second->GetInitPasses())
				{
					RenderingTileItem pass_info;
					pass_info.item_parent = new QTreeWidgetItem(passes);
					pass_info.item_parent->setText(0, p->GetName().c_str());
					pass_info.pass = p.get();
					//CPU
					{
						pass_info.item_cpu = new QTreeWidgetItem(pass_info.item_parent);
						pass_info.item_cpu->setText(0, "CPU");
					}
					//GPU
					{
						pass_info.item_gpu = new QTreeWidgetItem(pass_info.item_parent);
						pass_info.item_gpu->setText(0, "GPU");
					}
					_renderingTimes.push_back(pass_info);
				}
			}
		}
	}
}

PerformanceWatcher::~PerformanceWatcher()
{

}

void PerformanceWatcher::closeEvent(QCloseEvent* event)
{
	_timer->stop();
	SaveEditorWindowSetting(this, "PerformanceWatcher");
}

void PerformanceWatcher::UpdateAssetWatcher()
{
	for (int i = 0; i < (int)AssetType::MaxNum - 1; i++)
	{
		auto assets = ContentManager::Get()->GetAssets((AssetType)(i + 1));//0是unknow
		for (auto& a : assets)
		{
			auto infoPtr = a.second.get();
			auto it = _assets.find(infoPtr);
			bool bUpdate = false;
			AssetItem* target = nullptr;
			if (it == _assets.end() && a.second->GetSharedAssetObject(false))
			{
				QTreeWidgetItem* newItem = new QTreeWidgetItem(_assetWatcherTree->topLevelItem(i));
				AssetItem newAssetItem = {};
				newAssetItem.item = newItem;
				newAssetItem.object = a.second->GetSharedAssetObject(false);
				_assets.emplace(a.second.get(), newAssetItem);
				target = &_assets[a.second.get()];
			}
			else if (it != _assets.end() && it->second.object.expired())
			{
				_assetWatcherTree->topLevelItem(i)->removeChild(it->second.item);
				delete it->second.item;
				it->second.item = nullptr;
				_assets.erase(it);
			}
			else if (it != _assets.end() && !it->second.object.expired())
			{
				target = &it->second;
			}

			if (target)
			{
				int refCount = (a.second->GetRefCount() - 1);
				target->item->setText(0, a.second->displayName.c_str());
				if (target->object.lock()->IsResident())
				{
					QString text = QString::number(refCount) + QString::fromLocal8Bit("(常驻)");
					target->item->setText(1, text);
				}
				else if (target->object.lock()->IsSystemAsset())
				{
					QString text = QString::number(refCount) + QString::fromLocal8Bit("(系统常驻)");
					target->item->setText(1, text);
				}
				else
				{
					if (refCount > 0)
					{
						QString text = QString::number(refCount);
						target->item->setText(1, text);
					}
					else
					{
						QString text = QString::number(refCount) + QString::fromLocal8Bit("(可销毁)");
						target->item->setText(1, text);
					}
				}
				target->item->setText(2, a.second->virtualFilePath.c_str());
				target->item->setText(3, a.second->guid.str().c_str());
			}
		}
	}
}

void PerformanceWatcher::UpdateRenderingTimeWatcher()
{ 
	//Frame Update
	{
		_frame.item_parent->setText(1, QString::number(VulkanApp::GetFrameRate()));
		_frame.item_cpu->setText(1, QString::number(VulkanApp::GetMainForm()->renderer->GetCPURenderingTime()));
		_frame.item_gpu;
	}
	//Passes Update
	{
		for (auto& i : _renderingTimes)
		{
			i.item_cpu->setText(1, QString::number(i.pass->GetPassCPURenderingTime()));
			i.item_gpu->setText(1, QString::number(i.pass->GetPassGPURenderingTime()));
		}
	}
}

QTreeWidgetItem* PerformanceWatcher::AddTreeTopItem(QTreeWidget* tree, QString itemName)
{
	QTreeWidgetItem* item = new QTreeWidgetItem(tree);
	item->setText(0, itemName);
	tree->addTopLevelItem(item);
	return item;
}
