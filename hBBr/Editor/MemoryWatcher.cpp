#include "MemoryWatcher.h"
#include "qlabel.h"
#include "FormMain.h"
#include "EditorCommonFunction.h"
#include "AssetObject.h"
#include "VulkanObjectManager.h"
#if WIN32
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

MemoryWatcher::MemoryWatcher(QWidget *parent)
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
			if (VulkanApp::GetForms().size() <= 0)
				return;
			//
			// 更新内存使用情况
			{
				size_t memoryUsage = 0;
				#if WIN32
				ProcessMemoryCounters(memoryUsage);
				#endif
				QString text = QString::fromLocal8Bit("内存使用 : ") + QString::number(memoryUsage / 1024);
				text += QString(" kb(") + QString::number(memoryUsage / 1024 / 1024) + " mb)";
				ui.MemoryStaus->setText(text);
			}
			// GC倒计时
			{
				QString text = QString::fromLocal8Bit("GC剩余倒计时 : ") + QString::number(int(VulkanObjectManager::Get()->GetMaxGCTime() - VulkanObjectManager::Get()->GetGCTime())); 
				_gcStaus->setText(text);
			}
			//
			if (_tabWidget->currentIndex() == 0)
			{
				UpdateAssetWatcher();
			}		
		});
	_timer->start();

	LoadEditorWindowSetting(this, "MemoryWatcher");

	//Asset Watcher
	{
		_assetWatcher = new QWidget(this);
		_tabWidget->addTab(_assetWatcher, "Memory Watcher");
		QVBoxLayout* mianLayout = new QVBoxLayout(this);
		mianLayout->setContentsMargins(0,0,0,0);
		mianLayout->setSpacing(0);
		_assetWatcher->setLayout(mianLayout);
		_assetWatcherTree = new QTreeWidget(this);
		_assetWatcherTree->setHeaderLabels(
			{
		GetEditorInternationalization("MemoryWatcher","AssetWatch-Header0"),
		GetEditorInternationalization("MemoryWatcher","AssetWatch-Header1"),
		GetEditorInternationalization("MemoryWatcher","AssetWatch-Header2"),
		GetEditorInternationalization("MemoryWatcher","AssetWatch-Header3"),
			});
		mianLayout->addWidget(_assetWatcherTree);
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
}

MemoryWatcher::~MemoryWatcher()
{

}

void MemoryWatcher::closeEvent(QCloseEvent* event)
{
	_timer->stop();
	SaveEditorWindowSetting(this, "MemoryWatcher");
}

void MemoryWatcher::UpdateAssetWatcher()
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
