#include "DirtyAssetsManager.h"
#include "ContentManager.h"
#include "World.h"
#include "EditorCommonFunction.h"
class DurtyAssetItem :public QTreeWidgetItem
{
public:
	std::weak_ptr<AssetInfoBase> _asset;
	std::shared_ptr<World> _world;
	std::shared_ptr<Level> _level;
	DurtyAssetItem(QTreeWidget* view) :QTreeWidgetItem(view)
	{
	}
};

DirtyAssetsManager::DirtyAssetsManager(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.verticalLayout->setContentsMargins(8,8,8,8);
	setObjectName("DirtyAssetsManager");
	setAttribute(Qt::WidgetAttribute::WA_AlwaysStackOnTop);
	setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
	setWindowFlags(Qt::Window);
	setWindowFlags(Qt::Tool);
	//setWindowFlag(Qt::FramelessWindowHint);
	
	ui.SaveButton->setText(GetEditorInternationalization("DirtyAssetsManager", "SaveButton"));
	ui.CancelButton->setText(GetEditorInternationalization("DirtyAssetsManager", "CancelButton"));
	ui.ClearButton->setText(GetEditorInternationalization("DirtyAssetsManager", "ClearButton"));

	setFocusPolicy(Qt::FocusPolicy::StrongFocus);

	setWindowTitle(GetEditorInternationalization("DirtyAssetsManager", "BaseTitle0"));
	ui.treeWidget->setIndentation(2);
	ui.treeWidget->setColumnCount(3);
	ui.treeWidget->setHeaderLabels({
		GetEditorInternationalization("DirtyAssetsManager", "Name"),
		GetEditorInternationalization("DirtyAssetsManager", "Type"),
		GetEditorInternationalization("DirtyAssetsManager", "VirtualPath"),
		GetEditorInternationalization("DirtyAssetsManager", "Repository") 
		});


	auto dirtyAssets = ContentManager::Get()->GetDirtyAssets();
	auto dirtyWorlds = World::GetDirtyWorlds();
	auto dirtyLevels = Level::GetDirtyLevels();
	_allItems.clear();
	//第一行为功能item
	{
		//_headerItem = new DurtyAssetItem(ui.treeWidget);
		//ui.treeWidget->addTopLevelItem(_headerItem);
		//_headerItem->setCheckState(0, Qt::Checked);
		//_headerItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
		ui.SelectAllCheckBox->setCheckState(Qt::Checked);
	}

	for (auto& i : dirtyAssets)
	{
		if (!i.expired())
		{
			DurtyAssetItem* item = new DurtyAssetItem(ui.treeWidget);

			if(i.lock()->bDirtySelect)
				item->setCheckState(0, Qt::Checked);
			else
			{
				item->setCheckState(0, Qt::Unchecked);
				ui.SelectAllCheckBox->setCheckState(Qt::Unchecked);
			}
			item->_asset = i;
			//Column
			item->setText(0, i.lock()->displayName.c_str());//Name
			item->setText(1, (GetAssetTypeString(i.lock()->type)).c_str());//Type
			item->setText(2, i.lock()->virtualFilePath.c_str());//Virtual path
			item->setText(3, i.lock()->repository.c_str());//Repository

			ui.treeWidget->addTopLevelItem(item);
			_allItems.append(item);
		}
	}

	for (auto& i : dirtyWorlds)
	{
		if (!i.expired())
		{
			DurtyAssetItem* item = new DurtyAssetItem(ui.treeWidget);

			if (i.lock()->IsDirtySelect())
				item->setCheckState(0, Qt::Checked);
			else
			{
				item->setCheckState(0, Qt::Unchecked);
				ui.SelectAllCheckBox->setCheckState(Qt::Unchecked);
			}
			item->_world = i.lock();
			//Column
			item->setText(0, i.lock()->GetWorldName().c_str());//Name
			item->setText(1, "World");//Type
			item->setText(2, "");//Virtual path
			item->setText(3, "");

			ui.treeWidget->addTopLevelItem(item);
			_allItems.append(item);
		}
	}

	for (auto& i : dirtyLevels)
	{
		if (!i.expired())
		{
			DurtyAssetItem* item = new DurtyAssetItem(ui.treeWidget);

			if (i.lock()->IsDirtySelect())
				item->setCheckState(0, Qt::Checked);
			else
			{
				item->setCheckState(0, Qt::Unchecked);
				ui.SelectAllCheckBox->setCheckState(Qt::Unchecked);
			}
			item->_level = i.lock();
			//Column
			item->setText(0, i.lock()->GetLevelName().c_str());//Name
			item->setText(1, "Level");//Type
			item->setText(2, "");//Virtual path
			item->setText(3, "");

			ui.treeWidget->addTopLevelItem(item);
			_allItems.append(item);
		}
	}

	//设置排序属性
	ui.treeWidget->setSortingEnabled(true);

	// 设置默认排序列和顺序
	ui.treeWidget->sortByColumn(0, Qt::AscendingOrder);

	//	
	connect(ui.SaveButton, &QPushButton::clicked, this, [this, dirtyAssets]()
		{
			for (auto& i : _allItems)
			{
				if (i->checkState(0) == Qt::Checked)
				{
					if (!i->_asset.expired())
					{
						ContentManager::Get()->SaveAssetInfo(i->_asset);
						ContentManager::Get()->RemoveDirtyAsset(i->_asset);
					}
					else if (i->_world)
					{
						i->_world->SaveWorld();
						World::RemoveDirtyWorld(i->_world);
					}
					else if (i->_level)
					{
						i->_level->SaveLevel();
						for (auto& i : i->_level->GetDirtyFunc())
						{
							i();
						}
						Level::RemoveDirtyLevel(i->_level);
					}
				}
			}
			for (int i = 0; i < _finishExec.size(); i++)
			{
				_finishExec[i]();
			}
			close();
		});
	connect(ui.CancelButton, &QPushButton::clicked, this, [this]()
		{
			close();
		});

	connect(ui.ClearButton, &QPushButton::clicked, this, [this]()
		{
			ContentManager::Get()->ClearDirtyAssets();
			World::ClearDirtyWorlds();
			Level::ClearDirtyLevels();
			for (int i = 0; i < _finishExec.size(); i++)
			{
				_finishExec[i]();
			}
			close();
		});

	connect(ui.treeWidget, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* item, int column) 
		{
			auto di =(DurtyAssetItem*)item;
			if (di->checkState(0) == Qt::Checked && !di->_asset.expired())
			{
				di->_asset.lock()->bDirtySelect = true;
			}
			else if (di->checkState(0) == Qt::Unchecked && !di->_asset.expired())
			{
				di->_asset.lock()->bDirtySelect = false;
			}
			//
			if (di->checkState(0) == Qt::Checked && di->_world)
			{
				di->_world->bDirtySelect = true;
			}
			else if (di->checkState(0) == Qt::Unchecked && di->_world)
			{
				di->_world->bDirtySelect = false;
			}
			//
			if (di->checkState(0) == Qt::Checked && di->_level)
			{
				di->_level->bDirtySelect = true;
			}
			else if (di->checkState(0) == Qt::Unchecked && di->_level)
			{
				di->_level->bDirtySelect = false;
			}
			//
		});

	//    void itemChanged(QTreeWidgetItem *item, int column);
	connect(ui.SelectAllCheckBox, &QCheckBox::stateChanged, this, [this](int state)
		{
			if (state == Qt::Checked)
			{
				for (auto& i : _allItems)
				{
					i->setCheckState(0, Qt::Checked);
				}
			}
			else if (state == Qt::Unchecked)
			{
				for (auto& i : _allItems)
				{
					i->setCheckState(0, Qt::Unchecked);
				}
			}
		});

}

DirtyAssetsManager::~DirtyAssetsManager()
{

}

int DirtyAssetsManager::exec()
{
	if (_allItems.size() <= 0)
	{
		close();
		return -10;
	}

	int w = 800, h = 600;
	int wx = -1, wy = -1;
	GetEditorInternationalizationInt("DirtyAssetsManager", "WindowWidth", w);
	GetEditorInternationalizationInt("DirtyAssetsManager", "WindowHeight", h);
	GetEditorInternationalizationInt("DirtyAssetsManager", "PosX", wx);
	GetEditorInternationalizationInt("DirtyAssetsManager", "PosY", wy);
	if (wx < 0)
		wx = x();
	if (wy < 0)
		wy = y();
	resize(w, h);
	move(wx, wy);
	return QDialog::exec();
}

void DirtyAssetsManager::paintEvent(QPaintEvent* event)
{
	QDialog::paintEvent(event);
}

void DirtyAssetsManager::closeEvent(QCloseEvent* e)
{
	SetEditorInternationalizationInt("DirtyAssetsManager", "WindowWidth", this->width());
	SetEditorInternationalizationInt("DirtyAssetsManager", "WindowHeight", this->height());
	SetEditorInternationalizationInt("DirtyAssetsManager", "PosX", this->x());
	SetEditorInternationalizationInt("DirtyAssetsManager", "PosY", this->y());
}
