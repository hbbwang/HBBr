#include "ContentBrowser.h"
#include <QStyleOption>
#include <qpainter.h>
#include <qsplitter.h>
#include <qtreeview.h>
#include <qlistview.h>
#include <QFileSystemModel>
#include <qheaderview.h>
#include <QMouseEvent>
#include <qdebug.h>
#include <QMessageBox>
#include <QAction>
#include <QPushButton>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <qmimedata.h>
#include "CustomSearchLine.h"
#include "EditorCommonFunction.h"
#include "HString.h"
#include <QApplication>
#include <QFileDialog>
#include "QDesktopServices.h"
#include "qabstractitemview.h"
#include "FileSystem.h"
#include "RendererCore/Core/VulkanRenderer.h"
#include "RendererCore/Form/FormMain.h"
#include "Asset/Material.h"
#include "qdir.h"
#include "ComboBox.h"
#include "EditorMain.h"
#include "ConsoleDebug.h"
#include "MaterialDetailEditor.h"
#include "NvidiaTextureTools.h"
#include <QStyledItemDelegate>
#include <QStyledItemDelegate>

//--------------------------------------VirtualFolderTreeView-------------------
#pragma region VirtualFolderTreeView
void GetAllFolderChildItems(CustomViewItem* parentItem , QList<CustomViewItem*> & out)
{
	if (parentItem)
	{
		out.append(parentItem);
		ConsoleDebug::printf_endl("Get Virtual Folder : %s", parentItem->text().toStdString().c_str());
		int rowCount = parentItem->rowCount();
		for (int row = 0; row < rowCount; ++row)
		{
			CustomViewItem* childItem = (CustomViewItem*)parentItem->child(row);
			if (childItem)
			{
				ConsoleDebug::printf_endl("Get Virtual Folder : %s", childItem->text().toStdString().c_str());
				// 递归获取子项的子项
				GetAllFolderChildItems(childItem,out);
			}
		}
	}
}

class VirtualFolderTreeViewDelegate : public QStyledItemDelegate
{
public:
	class VirtualFolderTreeView* _tree;
	VirtualFolderTreeViewDelegate(VirtualFolderTreeView* parent) :QStyledItemDelegate(parent)
	{
		_tree = parent;
	}
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		QStyleOptionViewItem opt = option;
		initStyleOption(&opt, index);
		//CustomViewItem* item = (CustomViewItem*)((QStandardItemModel*)_tree->model())->itemFromIndex(index);
		auto rect = opt.rect;
		QStyledItemDelegate::paint(painter, opt, index);
		if (index.isValid())
		{
			if (_tree->isExpanded(index))
			{
				QPixmap eye((FileSystem::GetConfigAbsPath() + "Theme/Icons/ContentBrowser_FolderOpen.png").c_str());
				eye.scaled(rect.height() - 1, rect.height() - 1);
				painter->drawPixmap(rect.x() - rect.height() - 1, rect.y(), rect.height(), rect.height(), eye);
			}
			else
			{
				QPixmap eye((FileSystem::GetConfigAbsPath() + "Theme/Icons/ContentBrowser_FolderClose.png").c_str());
				eye.scaled(rect.height() - 1, rect.height() - 1);
				painter->drawPixmap(rect.x() - rect.height() - 1, rect.y(), rect.height(), rect.height(), eye);
			}
		}
	}
};

VirtualFolderTreeView::VirtualFolderTreeView(class  ContentBrowser* contentBrowser, QWidget* parent)
	:CustomTreeView(parent)
{
	setObjectName("CustomTreeView_VirtualFolderTreeView");
	setIndentation(20);
	//setEditTriggers(EditTrigger::DoubleClicked);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	_contentBrowser = contentBrowser;
	_newSelectionItems.reserve(50);
	_currentSelectionItem = 0;
	_bAddSelectionItem = _bSubSelectionItem = false;

	setItemDelegate(new VirtualFolderTreeViewDelegate(this));

	setDragEnabled(true);
	setAcceptDrops(true);
	//Context Menu
	{
		setContextMenuPolicy(Qt::ContextMenuPolicy::DefaultContextMenu);
		_contextMenu = new QMenu(this);
		QAction* importAssets = new QAction(GetEditorInternationalization("ContentBrowser","ImportAssets"), _contextMenu);
		QAction* createFolder = new QAction(GetEditorInternationalization("ContentBrowser", "CreateFolder"), _contextMenu);
		QAction* deleteFile = new QAction(GetEditorInternationalization("ContentBrowser", "DeleteFolder"), _contextMenu);
		QAction* rename = new QAction(GetEditorInternationalization("ContentBrowser", "FolderRename"), _contextMenu);
		_contextMenu->addAction(importAssets);
		_contextMenu->addAction(createFolder);
		_contextMenu->addAction(rename);
		_contextMenu->addSeparator();
		_contextMenu->addAction(deleteFile);
		//资产导入
		ActionConnect(importAssets, [this]()
			{
				_contentBrowser->ImportAssets();
			});
		//创建一个新的虚拟文件夹
		ActionConnect(createFolder, [this]() 
			{
				if (currentIndex().isValid())
				{
					auto itemModel = ((QStandardItemModel*)model());
					CreateNewVirtualFolder((CustomViewItem*)itemModel->itemFromIndex(currentIndex()));
				}
			});
		//删除当前虚拟文件夹内的所有文件
		ActionConnect(deleteFile, [this]() 
			{
				//收集当前选中的虚拟目录 + 它们的所以子目录
				QList<CustomViewItem*> allFoldersForDelete; 
				{
					auto items = GetSelectionItems();
					for (auto& i : items)
					{
						GetAllFolderChildItems(i, allFoldersForDelete);
					}
				}
				DeleteFolders(allFoldersForDelete);

			});
		//给虚拟文件夹重命名
		ActionConnect(rename, [this]()
			{
				//资产改名
				auto items = GetSelectionItems();
				if (items.size() > 0)
				{
					_ediingItem = items[0];
					edit(_ediingItem->index());
				}
			});
	}
	connect(model(), &QAbstractItemModel::dataChanged, this, &VirtualFolderTreeView::onDataChanged);
}

void VirtualFolderTreeView::AddItem(CustomViewItem* newItem, CustomViewItem* parent)
{
	if (!newItem)
	{
		return;
	}
	newItem->_text = newItem->text();
	if (parent)
	{
		newItem->_path = parent->_path + "/" + parent->_text;
		newItem->_fullPath = newItem->_path + "/" + newItem->_text;
		parent->appendRow(newItem);
	}
	else
	{
		//newItem->_path = parent->_path + "/" + _rootItem->_text;
		newItem->_fullPath = "/" + newItem->_text;
		((QStandardItemModel*)model())->appendRow(newItem);
	}
	_allItems.append(newItem);
}

CustomViewItem* VirtualFolderTreeView::FindFolder(QString virtualPath)
{
	//auto folderName = FileSystem::GetFileName(virtualPath.toStdString().c_str());
	//FindItem
	for (auto& i : _allItems)
	{
		QString shortPath = i->_path + i->_text;
		shortPath = shortPath.replace("/", "");
		shortPath = shortPath.replace("\\", "");
		QString vp = virtualPath;
		vp = vp.replace("/", "");
		vp = vp.replace("\\", "");
		if (shortPath.compare(vp) == 0)
		{
			return i;
		}
	}
	return nullptr;
}

void VirtualFolderTreeView::SelectionItem(QString vPath)
{
	CustomViewItem* item = nullptr;
	while (item==nullptr)
	{
		item = FindFolder(vPath);
		if (item)
		{
			break;
		}
		else
		{
			vPath = FileSystem::GetFilePath(vPath.toStdString().c_str()).c_str();
		}
	}
	if (item)
	{
		selectionModel()->clearSelection();
		selectionModel()->setCurrentIndex(item->index(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
	}
}

//点击TreeView之后更新FileListView
void VirtualFolderTreeView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	_contentBrowser->RefreshFileOnListView(false);
	CustomTreeView::selectionChanged(selected, deselected);
}

void VirtualFolderTreeView::DeleteFolders(QList<CustomViewItem*> allFoldersForDelete)
{
	//获取这些虚拟目录资产,删除
	HString msg;
	int count = 0;
	std::vector<AssetInfoBase*>assetsForDelete;
	for (auto& i : allFoldersForDelete)
	{
		auto assets = ContentManager::Get()->GetAssetsByVirtualFolder(i->_fullPath.toStdString().c_str());
		for (auto& a : assets)
		{
			if (assetsForDelete.capacity() <= assetsForDelete.size())
			{
				assetsForDelete.reserve(assetsForDelete.capacity() + 100);
			}
			assetsForDelete.push_back(a.second.get());
		}
		if (count < 20)
		{
			HString newLine = i->_fullPath.toStdString().c_str();
			msg += newLine + "\n";
			count++;
		}
	}
	msg = HString("You are deleting vitrual folder, are you sure?\nNormally, this is not recommended.This will also delete all the files in it (including the subdirectories,this can result in irreversible data loss.\n\n") + msg;
	ConsoleDebug::print_endl(msg, "255,255,0");
	QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete virualt folder", msg.c_str(), QMessageBox::Yes | QMessageBox::Cancel);

	if (reply == QMessageBox::Yes)
	{
		ContentManager::Get()->AssetDelete(assetsForDelete, true, false);
		//刷新内容浏览器
		ContentBrowser::RefreshContentBrowsers();
	}

}

void VirtualFolderTreeView::mousePressEvent(QMouseEvent* event)
{
	CustomTreeView::mousePressEvent(event);

}

void VirtualFolderTreeView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
	CustomTreeView::currentChanged(current, previous);
	if(current != previous)
	{
		QStandardItemModel* Itemmodel = (QStandardItemModel*)model();
		CustomViewItem* item = (CustomViewItem*)(Itemmodel->itemFromIndex(current));
		CustomViewItem* lastItem = (CustomViewItem*)(Itemmodel->itemFromIndex(previous));
		if (item ==nullptr || item == lastItem)
		{
			return;
		}
		auto text = item->_fullPath;
		if (text[0] != '/')
		{
			text = "/" + text;
		}
		text = "Asset" + text.replace("/", " / ");
		_contentBrowser->ui.PathLabel->setText(text);


		{
			if(!_bAddSelectionItem && !_bSubSelectionItem)
			{
				for (int i = 0; i < _currentSelectionItem; i++)
				{
					_newSelectionItems.removeFirst();
				}
				_currentSelectionItem = 0;
				_newSelectionItems.insert(0, item->_fullPath);
				if (_newSelectionItems.size() > 50)
				{
					_newSelectionItems.removeLast();
				}
			}
			else if(_bAddSelectionItem)
			{
				_currentSelectionItem++;
				_currentSelectionItem = std::min(_newSelectionItems.size(), _currentSelectionItem);
			}
			else if (_bSubSelectionItem)
			{
				_currentSelectionItem--;
				_currentSelectionItem = std::max(0, _currentSelectionItem);
			}
		}
		_bAddSelectionItem = false;
		_bSubSelectionItem = false;
	}
}

void VirtualFolderTreeView::contextMenuEvent(QContextMenuEvent* event)
{
	CustomTreeView::contextMenuEvent(event);
	_contextMenu->exec(event->globalPos());
}

void VirtualFolderTreeView::dragEnterEvent(QDragEnterEvent* e)
{
	qDebug() << e->mimeData()->text();
	if (e->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))//如果是来自Item的数据
	{
		//QT 的Item，只支持来自ContentBrowser的拖拽
		QObject* par = e->source();int count =0;
		while(par)
		{
			if (par->objectName().contains("ContentBrowser", Qt::CaseInsensitive))
			{
				e->acceptProposedAction();
				break;
			}
			par = par->parent();
			count++; if (count > 10)break;
		}
	}
}

void VirtualFolderTreeView::FindAllFolders(CustomViewItem* item , QList<CustomViewItem*>lists)
{
	if (item->hasChildren())
	{
		for (int i = 0; i < item->rowCount(); i++)
		{
			auto child = (CustomViewItem*)(item->child(i));
			if (child)
			{
				lists.append(child);
				FindAllFolders(child, lists);
			}
		}
	}
}

void VirtualFolderTreeView::dropEvent(QDropEvent* e)
{
	bool bDeleteEmptyFolder = false;
	//Get Items
	{
		QByteArray encoded = e->mimeData()->data("application/x-qabstractitemmodeldatalist");
		CustomViewItem* target = nullptr;
		auto targetIndex = indexAt(e->pos());
		QStandardItemModel* model = (QStandardItemModel*)(this->model());
		if (targetIndex.isValid())
		{
			target = (CustomViewItem*)model->itemFromIndex(indexAt(e->pos()));
		}
		if (target && !encoded.isEmpty())
		{
			std::vector<std::weak_ptr<AssetInfoBase>> assets;
			std::vector<HString> newVirtualFolderPaths;
			QDataStream stream(&encoded, QIODevice::ReadOnly);
			while (!stream.atEnd())
			{
				if (assets.capacity() < assets.size())
				{
					assets.reserve(assets.capacity() + 25);
					newVirtualFolderPaths.reserve(assets.capacity() + 25);
				}
				int row, col;
				QMap<int, QVariant> roleDataMap;
				stream >> row >> col >> roleDataMap;
				if (roleDataMap.contains(Qt::DisplayRole))
				{
					//收集拖拽的AssetInfos
					if (e->source()->objectName().compare(this->objectName()) == 0)//Tree View
					{
						//QMessageBox::information(0, 0, QString::number(row), 0);
						//auto from = (CustomViewItem*)model->item(row);
						bDeleteEmptyFolder = true;
						auto selectItems = GetSelectionItems();
						clearSelection();

						HString msg = "You're moving the virtual directory, are you sure?";
						QMessageBox::StandardButton reply = QMessageBox::question(this, "Move folder", msg.c_str(), QMessageBox::Yes | QMessageBox::Cancel);
						if (reply == QMessageBox::Yes)
						{
							//遍历里面的子项
							for (auto& i : selectItems)
							{
								FindAllFolders(i, selectItems);
							}
							for (auto& i : selectItems)
							{
								auto folderAssets = ContentManager::Get()->GetAssetsByVirtualFolder(i->_fullPath.toStdString().c_str());
								for (auto& i : folderAssets)
								{
									assets.push_back(i.second);
									HString newPath = HString(target->_fullPath.toStdString().c_str()) + "/" + i.second->virtualPath.GetBaseName();
									newVirtualFolderPaths.push_back(newPath);
								}
							}
						}						
					}
					else if (e->source()->objectName().compare(_contentBrowser->_listView->objectName()) == 0)//List View
					{
						//QMessageBox::information(0, 0, QString::number(row), 0);
						auto from = ((CustomListItem*)_contentBrowser->_listView->item(row));
						if (from && !from->_assetInfo.expired())
						{
							assets.push_back(from->_assetInfo);
							newVirtualFolderPaths.push_back(target->_fullPath.toStdString().c_str());
						}
					}
				}
			}
			//
			if (assets.size() > 0)
			{
				for (int i = 0; i < newVirtualFolderPaths.size();i++)
				{
					ContentManager::Get()->SetNewVirtualPath({ assets[i] },  newVirtualFolderPaths[i], bDeleteEmptyFolder);
				}
			}
			ContentBrowser::RefreshContentBrowsers();
		}
	}
}

CustomViewItem* VirtualFolderTreeView::CreateNewVirtualFolder(CustomViewItem* parent, QString folderName)
{
	if (!parent)
	{
		return nullptr;
	}
	auto itemModel = ((QStandardItemModel*)model());
	if (parent->index().isValid())
	{
		CustomViewItem* currentItem = (CustomViewItem*)itemModel->itemFromIndex(parent->index());
		QString newName = folderName;
		int index = -1;
		while (true)
		{
			bool bFound = false;
			int rowCount = currentItem->rowCount();
			for (int row = 0; row < rowCount; ++row)
			{
				QStandardItem* childItem = currentItem->child(row);
				if (childItem->text().compare(newName) == 0)
				{
					bFound = true;
					index++;
					newName = folderName + "_" + QString::number(index);
					break;
				}
			}
			if (!bFound)
			{
				break;
			}
		}
		CustomViewItem* newItem = new CustomViewItem(newName);
		AddItem(newItem, currentItem);
		//
		selectionModel()->clearSelection();
		selectionModel()->setCurrentIndex(newItem->index(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
		return newItem;
	}
	return nullptr;
}

void VirtualFolderTreeView::onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
	auto itemModel = ((QStandardItemModel*)model());
	if (topLeft.isValid() && itemModel->itemFromIndex(topLeft))
	{
		CustomViewItem* currentItem = (CustomViewItem*)itemModel->itemFromIndex(topLeft);
		{
			//此函数是Item已经改完名字之后触发的，所有不需要创建新的目录.
			if (currentItem->text().length() <= 1)
			{
				ConsoleDebug::print_endl("Folder name length must be greater than 1","255,255,0");
			}
			QString newName = currentItem->text();
			int index = -1;
			while (true)
			{
				bool bFound = false;
				int rowCount = currentItem->rowCount();
				for (int row = 0; row < rowCount; ++row)
				{
					QStandardItem* childItem = currentItem->child(row);
					if (childItem->text().compare(newName) == 0)
					{
						bFound = true;
						index++;
						newName = currentItem->text() + "_" + QString::number(index);
						break;
					}
				}
				if (!bFound)
				{
					break;
				}
			}

			auto assets = ContentManager::Get()->GetAssetsByVirtualFolder(currentItem->_fullPath.toStdString().c_str());
			HString oldName = currentItem->_text.toStdString().c_str();
			currentItem->_text = currentItem->text();
			currentItem->_fullPath = currentItem->_path + "/" + currentItem->_text;
			// 
			//把资产都移动到新目录
			if (assets.size() > 0)
			{
				std::vector<std::weak_ptr<AssetInfoBase>> infos;
				infos.reserve(assets.size());
				for (auto& i : assets)
				{
					infos.push_back(i.second);
				}
				ContentManager::Get()->SetNewVirtualPath(infos, currentItem->_fullPath.toStdString().c_str());
			}
			else
			{
				ContentManager::Get()->CreateNewVirtualFolder(currentItem->_fullPath.toStdString().c_str());
			}
			// 
			ConsoleDebug::print_endl("Virtual folder rename : [" + oldName + "] to [" + currentItem->_text.toStdString().c_str() + "]");

			selectionModel()->clearSelection();
			selectionModel()->setCurrentIndex(currentItem->index(), QItemSelectionModel::SelectionFlag::ClearAndSelect);

			//更新
			ContentBrowser::RefreshContentBrowsers();		
		}	
	}
}
#pragma endregion
//--------------------------------------Virtual File List View-------------------
#pragma region VirtualFileListView
VirtualFileListView::VirtualFileListView(class  ContentBrowser* contentBrowser, QWidget* parent)
	:CustomListView(parent)
{
	_contentBrowser = contentBrowser;
	setObjectName("CustomListView_VirtualFileListView");
	setMouseTracking(true);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	//Context Menu
	{
		setContextMenuPolicy(Qt::ContextMenuPolicy::DefaultContextMenu);
		_contextMenu = new QMenu(this);
		QAction* importAssets = new QAction(GetEditorInternationalization("ContentBrowser", "ImportAssets"),_contextMenu);
		_contextMenu->addAction(importAssets);
		{
			QMenu* createFile = new QMenu("Create", _contextMenu);
			_contextMenu->addMenu(createFile);
			QAction* createMaterial = new QAction(GetEditorInternationalization("ContentBrowser", "CreateMaterialInstance"), createFile);
			createFile->addAction(createMaterial);
			ActionConnect(createMaterial, [this]()
				{
					auto repositorySelection = new RepositorySelection(this);
					repositorySelection->setObjectName("ContentBrowser_RepositorySelection");
					repositorySelection->_selectionCallBack = [this](QString repository)
					{
						if (_contentBrowser->_treeView->currentIndex().isValid())
						{
							CustomViewItem* treeItem = ((CustomViewItem*)((QStandardItemModel*)_contentBrowser->_treeView->model())->itemFromIndex(_contentBrowser->_treeView->currentIndex()));
							HString virtualPath = treeItem->_fullPath.toStdString().c_str();
							Material::CreateMaterial(repository.toStdString().c_str(), virtualPath);
							ContentBrowser::RefreshContentBrowsers();
						}
					};
					repositorySelection->exec();
				});
		}
		QAction* deleteFile = new QAction(GetEditorInternationalization("ContentBrowser", "DeleteFile"), _contextMenu);
		QAction* rename = new QAction(GetEditorInternationalization("ContentBrowser", "FileRename"), _contextMenu);
		_contextMenu->addAction(rename);
		_contextMenu->addSeparator();
		_contextMenu->addAction(deleteFile);
		ActionConnect(importAssets, [this]() 
			{
				_contentBrowser->ImportAssets();
			});
		ActionConnect(deleteFile, [this]()
			{
				HString msg;
				auto items = GetSelectionItems();
				if (items.size() > 0)
				{
					std::vector<AssetInfoBase*> infos;
					infos.reserve(items.size());
					for (auto& i : items)
					{
						if (!i->_assetInfo.expired())
						{
							infos.push_back(i->_assetInfo.lock().get());
							msg += i->_assetInfo.lock()->virtualFilePath + "\n";
						}
					}

					msg = HString("You are deleting assets, are you sure?\nPlease check the asset reference relationship before deleting.\n\n") + msg;
					ConsoleDebug::print_endl(msg, "255,255,0");
					QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete assets", msg.c_str(), QMessageBox::Yes | QMessageBox::Cancel);

					if (reply == QMessageBox::Yes)
					{
						ContentManager::Get()->AssetDelete(infos, false, false);
					}
					else
					{
					}

					//刷新内容浏览器
					ContentBrowser::RefreshContentBrowsers();
				}
			});
		ActionConnect(rename, [this]()
			{
				//资产改名
				auto items = GetSelectionItems();
				if (items.size() > 0)
				{
					_ediingItem = items[0];
					editItem(items[0]);
				}
			});

		connect(this,&QListWidget::itemChanged,this,&VirtualFileListView::ItemTextChange);
		connect(this, &QListWidget::itemDoubleClicked, this, &VirtualFileListView::ItemDoubleClicked);
	}

}

void VirtualFileListView::contextMenuEvent(QContextMenuEvent* event)
{
	CustomListView::contextMenuEvent(event);
	_contextMenu->exec(event->globalPos());
}

void VirtualFileListView::mouseMoveEvent(QMouseEvent* event)
{
	QPoint pos = mapFromGlobal(event->globalPos());
	CustomListItem* item = (CustomListItem*)this->itemAt(pos);
	if (item && item->_toolTip._tooltip.length() > 1)
	{
		if (_currentMouseTrackItem != item)
		{
			_currentMouseTrackItem = item;
		}
		_toolTipWidget->resize(0, 0);
		_toolTipWidget->setHidden(false);

		_toolTipLabel->setText(item->_toolTip._tooltip);
		_toolTipWidget->move(event->globalPos().x() + 10, event->globalPos().y() + 10);
	}
	else
	{
		_toolTipWidget->setHidden(true);
		_currentMouseTrackItem = nullptr;
	}
	QListWidget::mouseMoveEvent(event);
}

void VirtualFileListView::paintEvent(QPaintEvent* event)
{
	CustomListView::paintEvent(event);
}

void VirtualFileListView::dragEnterEvent(QDragEnterEvent* e)
{
	if (e->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))//如果是来自Item的数据
	{
		//QT 的Item，只支持来自ContentBrowser的拖拽
		QObject* par = e->source(); int count = 0;
		while (par)
		{
			if (par->objectName().contains("ContentBrowser", Qt::CaseInsensitive))
			{
				e->acceptProposedAction();
				break;
			}
			par = par->parent();
			count++; if (count > 10)break;
		}
	}
}

void VirtualFileListView::dropEvent(QDropEvent* e)
{
	//Get Items
	{
		QByteArray encoded = e->mimeData()->data("application/x-qabstractitemmodeldatalist");
		if (!encoded.isEmpty())
		{
			std::vector<AssetInfoBase*> assets;
			QDataStream stream(&encoded, QIODevice::ReadOnly);
			while (!stream.atEnd())
			{
				if (assets.capacity() < assets.size())
				{
					assets.reserve(assets.capacity() + 25);
				}
				int row, col;
				QMap<int, QVariant> roleDataMap;
				stream >> row >> col >> roleDataMap;
				if (roleDataMap.contains(Qt::DisplayRole))
				{
					//收集拖拽的AssetInfos
					if (e->source()->objectName().compare(this->objectName()) == 0)//List View
					{
						
					}
					else if (e->source()->objectName().compare(_contentBrowser->_treeView->objectName()) == 0)//Tree View
					{
						
					}
				}
			}
			ContentBrowser::RefreshContentBrowsers();
		}
	}
}

void VirtualFileListView::ItemDoubleClicked(QListWidgetItem* input_item)
{
	auto item = (CustomListItem*)input_item;
	if (!item->_assetInfo.expired())
	{
		switch (item->_assetInfo.lock()->type)
		{
		case AssetType::Material:
		{
			/*MaterialEditor::OpenMaterialEditor(item->_assetInfo.lock()->GetAssetObject<Material>(), true);*/
			auto mat = item->_assetInfo.lock()->GetAssetObject<Material>();
			MaterialDetailEditor::OpenMaterialEditor(mat);
			break;
		}

		default:
			break;
		}
	}
}

ToolTip VirtualFileListView::UpdateToolTips(std::weak_ptr<struct AssetInfoBase>& assetInfo)
{
	//收集资产的ToolTip
	ToolTip result;
	if (!assetInfo.expired())
	{
		for (auto& i : assetInfo.lock()->toolTips)
		{
			if (i.first.Length() > 1)
			{
				HString name = i.first;
				HString value = i.second;
				result._tooltip += (name + value).c_str();
				result._tooltip += "\n";
			}
		}
	}
	return result;
}

void VirtualFileListView::ItemTextChange(QListWidgetItem* widgetItem)
{
	//资产改名
	auto item = (CustomListItem*)widgetItem;
	if (!item->_assetInfo.expired() && _ediingItem == item)
	{
		if (!item->_assetInfo.lock()->displayName.IsSame(item->text().toStdString().c_str()))
		{
			HString newName = ContentManager::Get()->SetVirtualName(item->_assetInfo, item->text().toStdString().c_str());
			item->setText(newName.c_str());
			item->_toolTip = UpdateToolTips(item->_assetInfo);
			ContentManager::Get()->MarkAssetDirty(item->_assetInfo);
		}
		_ediingItem = nullptr;
	}
}

CustomListItem* VirtualFileListView::AddFile(std::weak_ptr<struct AssetInfoBase> assetInfo, bool bUpdatePreview)
{
	if (!assetInfo.expired())
	{
		//收集资产的ToolTip
		ToolTip newToolTip = UpdateToolTips(assetInfo);
		//生成一下预览图
		if (bUpdatePreview)
		{
			SpawnAssetPreviewImage(assetInfo);
		}
		//目录下的jpg格式图像作为预览图存在
		auto	iconPath = (FileSystem::Append(FileSystem::GetAssetAbsPath(), "Saved/PreviewImage/Content/" + assetInfo.lock()->repository + "/Texture2D/" + assetInfo.lock()->guid.str() + ".jpg"));;
		if (!FileSystem::FileExist(iconPath))
		{
			iconPath = FileSystem::GetConfigAbsPath();
			if (assetInfo.lock()->type == AssetType::Model)
			{
				iconPath += "Theme/Icons/ICON_FILE_MODEL.png";
			}
			else if (assetInfo.lock()->type == AssetType::Material)
			{
				iconPath += "Theme/Icons/ICON_FILE_MAT.png";
			}
			else
			{
				iconPath += "Theme/Icons/ICON_FILE.png";
			}
		}
		auto newItem = AddItem(assetInfo.lock()->displayName.c_str(), iconPath.c_str(), newToolTip);
		newItem->_assetInfo = assetInfo;
		newItem->_path = assetInfo.lock()->virtualPath.c_str();
		newItem->_fullPath = assetInfo.lock()->virtualFilePath.c_str();
		return newItem;
	}
	return nullptr;
}

void VirtualFileListView::SpawnAssetPreviewImage(std::weak_ptr<struct AssetInfoBase> assetInfo)
{
	if (assetInfo.lock()->type == AssetType::Texture2D)
	{
		HString previewPath = (FileSystem::Append(FileSystem::GetAssetAbsPath(), "Saved/PreviewImage/Content/" + assetInfo.lock()->repository + "/Texture2D/" + assetInfo.lock()->guid.str() + ".jpg"));
		NVTT::DecompressionImage2D(
			assetInfo.lock()->absFilePath.c_str(),
			previewPath.c_str(),
			nullptr, 64, 64);
	}
}

QList<CustomListItem*> VirtualFileListView::FindItems(QString itemPath)
{
	return CustomListView::FindItems(itemPath);
}

CustomListItem* VirtualFileListView::FindItem(QString itemPath)
{
	return CustomListView::FindItem(itemPath);
}

CustomListItem* VirtualFileListView::FindAssetItem(HGUID guid)
{
	CustomListItem* result = nullptr;
	for (auto & i : _allItems) 
	{
		if (!i->_assetInfo.expired())
		{
			if (i->_assetInfo.lock()->guid == guid)
			{
				return  i;
			}
		}
	}
	return nullptr;
}

CustomListItem* VirtualFileListView::FindAssetItem(QString assetName)
{
	for (auto& i : _allItems)
	{
		if (!i->_assetInfo.expired())
		{
			if (i->_assetInfo.lock()->displayName.IsSame(assetName.toStdString()))
			{
				return i;
			}
		}
	}
	return nullptr;
}

#pragma endregion
//--------------------------------------Repository Selection Widget-------------------
#pragma region RepositorySelectionWidget
RepositorySelection::RepositorySelection(QWidget* parent) :QDialog(parent)
{
	setAttribute(Qt::WidgetAttribute::WA_AlwaysStackOnTop);
	setWindowFlags(Qt::Window);
	setWindowFlags(Qt::Tool);
	setWindowFlag(Qt::FramelessWindowHint);
	//setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
	setFocusPolicy(Qt::FocusPolicy::StrongFocus);
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(8,8, 10, 8);
	setLayout(layout);

	_currentRepositorySelection = "?";

	combo = new ComboBox("RepositorySelection : ", this);
	layout->addWidget(combo);
	{
		QHBoxLayout* hLayout = new QHBoxLayout(this);
		QPushButton* bOK = new QPushButton("OK", this);
		QPushButton* bCancel = new QPushButton("Cancel", this);
		hLayout->addWidget(bOK);
		hLayout->addWidget(bCancel);
		layout->addLayout(hLayout);
		_selectionCallBack = [](QString t) {};
		connect(bOK, &QPushButton::clicked, this, [this]() {
			if (_currentRepositorySelection[0] == '?')
			{
				QMessageBox::information(0, 0, "Please select a repository", 0);
				return;
			}
			ConsoleDebug::printf_endl("Import assets to repository:%s", _currentRepositorySelection.toStdString().c_str());
			_selectionCallBack(_currentRepositorySelection);
			close();
		});
		connect(bCancel, &QPushButton::clicked, this, [this]() {
			close();
		});
	}

	combo->_bindCurrentTextChanged = [this](const int index, const char* text) 
	{
		_currentRepositorySelection = text;
	};
	combo->ui.horizontalLayout->setContentsMargins(1, 1, 1, 1);
	//combo->setMaximumHeight(50);
	combo->ui.ComboBox_0->setMaximumHeight(40);
	combo->ui.Name->setMaximumHeight(45);
}

void RepositorySelection::showEvent(QShowEvent*e)
{
	QDialog::showEvent(e);
	resize(300, 0);
	QPoint pos = QPoint(EditorMain::_self->x() + EditorMain::_self->width() / 2, EditorMain::_self->y() + EditorMain::_self->height() / 2);
	pos -= QPoint(width() / 2, height() / 2);
	//pos = EditorMain::_self->mapToGlobal(pos);

	setGeometry(pos.x(), pos.y(), 400, 0);
	setHidden(false);
	//
	combo->ClearItems();
	auto repositories = ContentManager::Get()->GetRepositories();
	for (auto& i : repositories)
	{
		HString name = i.first;
		combo->AddItem(name.c_str());
	}
	combo->SetCurrentSelection(_currentRepositorySelection);
	setFocus();
}

void RepositorySelection::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QStyleOption styleOpt;
	styleOpt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);
	QWidget::paintEvent(event);
}

void RepositorySelection::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
}

#pragma endregion
//--------------------------------------Content Browser Widget-------------------
#pragma region ContentBrowserWidget
QList<ContentBrowser*>ContentBrowser::_contentBrowser;
class ContentBrowser* ContentBrowser::_currentBrowser = nullptr;
ContentBrowser::ContentBrowser(QWidget* parent )
	:QWidget(parent)
{
	this->setObjectName("ContentBrowser");
	ui.setupUi(this);
	ui.horizontalLayout_2->setSpacing(2);
	ui.horizontalLayout_2->setContentsMargins(1, 1, 1, 1);
	ui.ContentBrowserVBoxLayout->setSpacing(1);
	ui.ContentBrowserVBoxLayout->setContentsMargins(1, 1, 1, 1);

	ui.horizontalLayout_2->setObjectName("ContentBrowser");
	ui.horizontalLayout_2->setSizeConstraint(QLayout::SetMinimumSize);

	ui.ImportButton->setObjectName("ContentBrowser_Button");
	ui.FrontspaceButton->setObjectName("ContentBrowser_Button");
	ui.BackToParentButton->setObjectName("ContentBrowser_Button");
	ui.BackspaceButton->setObjectName("ContentBrowser_Button");
	ui.SaveButton->setObjectName("ContentBrowser_Button");
	ui.OptionButton->setObjectName("ContentBrowser_Button");

	ui.ImportButton->setText(GetEditorInternationalization("ContentBrowser","ImportButton"));
	ui.SaveButton->setText(GetEditorInternationalization("ContentBrowser", "SaveButton"));

	ui.ImportButton->setMinimumWidth(1);
	ui.SaveButton->setMinimumWidth(1);
	ui.BackToParentButton->setMinimumWidth(1);
	ui.BackspaceButton->setMinimumWidth(1);
	ui.FrontspaceButton->setMinimumWidth(1);
	ui.OptionButton->setMinimumWidth(1);

	_splitterBox = new QSplitter(Qt::Horizontal, this);
	_splitterBox->setObjectName("ContentBrowser_Splitter");
	//
	_treeWidget = new QWidget(_splitterBox);
	_listWidget = new QWidget(_splitterBox);
	_splitterBox->addWidget(_treeWidget);
	_splitterBox->addWidget(_listWidget);
	QVBoxLayout* treeLayout = new QVBoxLayout(_treeWidget);
	QVBoxLayout* listLayout = new QVBoxLayout(_listWidget);
	treeLayout->setSpacing(1);
	treeLayout->setContentsMargins(1, 1, 1, 1);
	listLayout->setSpacing(1);
	listLayout->setContentsMargins(1, 1, 1, 1);
	//
	_treeView = new VirtualFolderTreeView(this, this);
	treeLayout->addWidget(_treeView);
	_listView = new VirtualFileListView(this, this);
	listLayout->addWidget(_listView);
	//
	_splitterBox->setStretchFactor(1, 4);
	ui.ContentBrowserVBoxLayout->addWidget(_splitterBox);
	ui.ContentBrowserVBoxLayout->setStretch(0,0);
	ui.ContentBrowserVBoxLayout->setStretch(1, 1000);
	
	//Path label
	ui.PathLabel->setObjectName("ContentBrowser_PathLabel");
	ui.PathLabel->setText("Asset /");

	_contentBrowser.append(this);
	_currentBrowser = this;
	//Connect
	connect(_treeView, &QTreeView::clicked, this, [this]() {
		if (_treeView->_currentSelectionItem != 0 && _treeView->_newSelectionItems.size() > 0)
		{
			_treeView->_newSelectionItems.erase(
				_treeView->_newSelectionItems.begin(), 
				_treeView->_newSelectionItems.end() - (_treeView->_newSelectionItems.size() - _treeView->_currentSelectionItem));
			_treeView->_currentSelectionItem = 0;
		}
	});
	//资产导入按钮
	connect(ui.ImportButton, &QAbstractButton::clicked, this, [this]() {
		ImportAssets();
	}); 
	//回到下一个文件夹 →
	connect(ui.FrontspaceButton, &QAbstractButton::clicked, this, [this]() {
		if (_treeView->_newSelectionItems.size() > 0 && _treeView->_currentSelectionItem > 0)
		{
			auto itemFullPath = _treeView->_newSelectionItems[_treeView->_currentSelectionItem - 1];
			auto item = _treeView->FindFolder(itemFullPath);
			{
				_treeView->_bSubSelectionItem = true;
				_treeView->selectionModel()->setCurrentIndex(item->index(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
			}
		}
	});
	//回到上一个文件夹 ←
	connect(ui.BackspaceButton, &QAbstractButton::clicked, this, [this]() {
		if (_treeView->_newSelectionItems.size() > 0 && _treeView->_newSelectionItems.size() > _treeView->_currentSelectionItem + 1)
		{
			auto itemFullPath = _treeView->_newSelectionItems[_treeView->_currentSelectionItem + 1];
			auto item = _treeView->FindFolder(itemFullPath);
			{
				_treeView->_bAddSelectionItem = true;
				_treeView->selectionModel()->setCurrentIndex(item->index(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
			}
		}
	});
	//回到当前的父文件夹 ↑

	ui.BackToParentButton->setHidden(true);
	/*connect(ui.BackToParentButton, &QAbstractButton::clicked, this, [this]() {

		if (_treeView->_newSelectionItems.size() > 0)
		{
			if (_treeView->currentIndex().isValid())
			{
				QStandardItemModel* model = (QStandardItemModel*)_treeView->model();
				CustomViewItem* item = (CustomViewItem*)model->itemFromIndex(_treeView->currentIndex());
				if (item && item->parent() && item->parent()->index().isValid())
				{
					_treeView->selectionModel()->setCurrentIndex(item->parent()->index(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
				}
			}
		}
	});*/

	//保存按钮
	connect(ui.SaveButton, &QAbstractButton::clicked, this, [this]() {
		EditorMain::_self->ShowDirtyAssetsManager();
	});
	//设置选项
	//ui.OptionButton->setIcon(QIcon((FileSystem::GetConfigAbsPath() + "Theme/Icons/ICON_OPTION.png").c_str()));
	_refreshContentBrowser = new QAction(GetEditorInternationalization("ContentBrowser", "RefreshContentBrowser"), this);
	_updateFileListPreviewImage = new QAction(GetEditorInternationalization("ContentBrowser", "UpdateFileListPreviewImage"), this);
	ActionConnect(_refreshContentBrowser, [this]() { ContentBrowser::RefreshContentBrowsers(); });
	ActionConnect(_updateFileListPreviewImage, [this]() { 
			this->RefreshFileOnListView(true);
		});
	_cbOptionMenu = new QMenu(this);
	_cbOptionMenu->addAction(_refreshContentBrowser);
	_cbOptionMenu->addAction(_updateFileListPreviewImage);
	connect(ui.OptionButton, &QAbstractButton::clicked, this, [&, this]()
		{
			auto pos = QPoint(0, 0 + ui.OptionButton->height());
			pos = ui.OptionButton->mapToGlobal(pos);
			_cbOptionMenu->exec(pos);
		});
	
	//Refresh
	RefreshContentBrowsers();
}

ContentBrowser::~ContentBrowser()
{
	_contentBrowser.removeOne(this);
	if (_currentBrowser == this)
	{
		_currentBrowser = nullptr;
	}
	if (_contentBrowser.size() > 0)
	{
		_currentBrowser = _contentBrowser[0];
	}
}

void ContentBrowser::RefreshContentBrowsers()
{
	for (auto& i : _contentBrowser)
	{
		i->Refresh();
	}
}

void ContentBrowser::Refresh()
{
	RefreshFolderOnTreeView();
	//默认选择root(Asset)根节点。不需要显示任何文件夹
}

void ContentBrowser::RefreshFolderOnTreeView()
{
	QString currentItemVPath;
	if (_treeView->GetSelectionItems().size() > 0)
	{
		currentItemVPath = ((CustomViewItem*)(_treeView->GetSelectionItems()[0]))->_fullPath;
	}

	_treeView->RemoveAllItems();
	auto folders = ContentManager::Get()->GetVirtualFolders();
	CustomViewItem* firstChoose = nullptr;
	for (auto& i : folders)
	{
		CustomViewItem* parent = nullptr;
		auto pathTag = i.second.Path.Split("/");
		for (int i = 0; i < pathTag.size(); i++)
		{
			QString vfp;
			if (parent)
			{
				vfp = parent->_path + "/" + parent->_text + "/";
			}
			vfp +=( pathTag[i]+ "/" ).c_str();
			CustomViewItem* newItem = _treeView->FindFolder(vfp);
			if (newItem == nullptr)
			{
				newItem = new CustomViewItem(pathTag[i].c_str());
				_treeView->AddItem(newItem, parent);
			}
			parent = newItem;
			if (firstChoose == nullptr)
			{
				firstChoose = parent;
			}
		}
	}
	{
		if (currentItemVPath.length() > 1)
		{
			_treeView->SelectionItem(currentItemVPath);
		}
		else
		{
			_treeView->selectionModel()->clearSelection();
			if(firstChoose)
				_treeView->selectionModel()->setCurrentIndex(firstChoose->index(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
		}
		_treeView->expand(_treeView->currentIndex());
	}
}

//更新FileListView的显示
void ContentBrowser::RefreshFileOnListView(bool bUpdatePreview)
{
	_listView->RemoveAllItems();
	auto item = _treeView->GetSelectionItems();
	_listView->_currentVirtualFolderItems.clear();
	for (auto& i : item)
	{
		auto assets = ContentManager::Get()->GetAssetsByVirtualFolder(i->_fullPath.toStdString().c_str());
		for (auto& a : assets)
		{
			auto item = _listView->AddFile(a.second, bUpdatePreview);
			_listView->_currentVirtualFolderItems.append(item);
		}
	}
}

void ContentBrowser::ShowRepositorySelection()
{
	auto new_rs = new RepositorySelection(this);
	new_rs->setObjectName("ContentBrowser_RepositorySelection");
	_Sleep(10);
	new_rs->_selectionCallBack = [this](QString repository)
	{
		//资产导入操作在这
		if (_importFileNames.size() > 0 && _treeView->currentIndex().isValid())
		{
			std::vector<AssetImportInfo> infos;
			infos.reserve(_importFileNames.size());
			for (auto& i : _importFileNames)
			{

				//QMessageBox::information(0,0,i,0);
				QFileInfo info(i);
				if (info.isDir()) continue;
				else
				{
					AssetImportInfo newInfo;
					newInfo.absAssetFilePath = i.toStdString().c_str();
					newInfo.virtualPath = ((CustomViewItem*)((QStandardItemModel*)_treeView->model())->itemFromIndex(_treeView->currentIndex()))->_fullPath.toStdString().c_str();
					infos.push_back(newInfo);
				}
			}
			std::vector<std::weak_ptr<AssetInfoBase>>results;
			bool bSucceed = ContentManager::Get()->AssetImport(repository.toStdString().c_str(), infos, &results);

			//生成预览图
			for (auto& i : results)
			{
				_listView->SpawnAssetPreviewImage(i);
			}
			
			_importFileNames.clear();
			if (bSucceed)
			{
				RefreshContentBrowsers();
			}
		}
	};
	new_rs->exec();
}

void ContentBrowser::ImportAssets()
{
	// 创建一个支持多选的文件选择对话框
	QStringList fileNames = QFileDialog::getOpenFileNames(nullptr,
		"Import Resources",  // 对话框标题
		"",         // 对话框初始目录
		"All File (*);;\
				Model (*.fbx);;\
				Image (*.png *.jpg *.bmp *.tga *.hdr);;\
			");
	ConsoleDebug::print_endl("Begin import assets...");
	if (fileNames.size() > 0)
	{
		_importFileNames = fileNames;
		ShowRepositorySelection();
	}
}

void ContentBrowser::FocusToAsset(std::weak_ptr<AssetInfoBase> assetInfo, ContentBrowser* cb)
{
	if (cb == nullptr)
	{
		cb = ContentBrowser::GetCurrentBrowser();
	}
	if (!assetInfo.expired())
	{
		auto guid = assetInfo.lock()->guid;
		auto vPath = assetInfo.lock()->virtualPath;
		auto treeItems = vPath.Split("/");
		QString path;
		CustomViewItem* treeItem = nullptr;
		QModelIndex treeIndex;
		for (auto& i : treeItems)
		{
			path += ("/" + i).c_str();
			treeItem = cb->_treeView->FindItem(path);
			if (treeItem)
			{
				treeIndex = ((QStandardItemModel*)cb->_treeView->model())->indexFromItem(treeItem);
				cb->_treeView->expand(treeIndex);
			}
		}
		if (treeIndex.isValid())
		{
			cb ->_treeView->selectionModel()->setCurrentIndex(treeIndex, QItemSelectionModel::SelectionFlag::ClearAndSelect);
		}
		auto item = cb->_listView->FindAssetItem(guid);
		if (item)
		{
			cb->_listView->scrollToItem(item);
			cb->_listView->setCurrentItem(item, QItemSelectionModel::SelectionFlag::ClearAndSelect);
		}
	}
}

void ContentBrowser::focusInEvent(QFocusEvent* event)
{
	QWidget::focusInEvent(event);
}

void ContentBrowser::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
}

void ContentBrowser::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QStyleOption styleOpt;
	styleOpt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);	
}

void ContentBrowser::mouseMoveEvent(QMouseEvent* event)
{
	QWidget::mouseMoveEvent(event);
}

void ContentBrowser::mousePressEvent(QMouseEvent* event)
{
	QWidget::mousePressEvent(event);
}

void ContentBrowser::closeEvent(QCloseEvent* event)
{
	QWidget::closeEvent(event);
}


#pragma endregion

