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
//--------------------------------------VirtualFolderTreeView-------------------
#pragma region VirtualFolderTreeView
VirtualFolderTreeView::VirtualFolderTreeView(class  ContentBrowser* contentBrowser, QWidget* parent)
	:CustomTreeView(parent)
{
	setObjectName("CustomTreeView_VirtualFolderTreeView");
	_rootItem->setText("Asset");
	_rootItem->_path = "";
	_rootItem->_text = "";
	setIndentation(20);
	setEditTriggers(EditTrigger::DoubleClicked);
	_contentBrowser = contentBrowser;
	_newSelectionItems.reserve(50);
	_currentSelectionItem = 0;
	_bSaveSelectionItem = true;
}

void VirtualFolderTreeView::AddItem(CustomViewItem* newItem, CustomViewItem* parent)
{
	if (!newItem)
	{
		return;
	}
	newItem->_text = newItem->text();
	if (parent && parent != _rootItem)
	{
		newItem->_path = parent->_path + "/" + parent->_text;
		newItem->_fullPath = newItem->_path + "/" + newItem->_text;
		parent->appendRow(newItem);
	}
	else
	{
		//newItem->_path = parent->_path + "/" + _rootItem->_text;
		newItem->_fullPath = newItem->_text;
		_rootItem->appendRow(newItem);
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

void VirtualFolderTreeView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	_contentBrowser->RefreshFileOnListView();
	CustomTreeView::selectionChanged(selected, deselected);
}

void VirtualFolderTreeView::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
	CustomTreeView::currentChanged(current, previous);
	if(current != previous)
	{
		QStandardItemModel* Itemmodel = (QStandardItemModel*)model();
		CustomViewItem* item = (CustomViewItem*)(Itemmodel->itemFromIndex(current));
		CustomViewItem* lastItem = (CustomViewItem*)(Itemmodel->itemFromIndex(previous));
		if (item == lastItem)
		{
			return;
		}
		auto text = item->_fullPath;
		if (text[0] != '/')
		{
			text = "/" + text;
		}
		text = "Asset" + text.replace("/", " - ");
		_contentBrowser->ui.PathLabel->setText(text);

		if (_bSaveSelectionItem)
		{
			_newSelectionItems.insert(0, item);
			if (_newSelectionItems.size() > 50)
			{
				_newSelectionItems.removeLast();
			}
		}
	}
}

#pragma endregion


//--------------------------------------Virtual File List View-------------------
#pragma region VirtualFileListView
VirtualFileListView::VirtualFileListView(QWidget* parent)
	:CustomListView(parent)
{
	setObjectName("CustomListView_VirtualFileListView");
	setMouseTracking(true);
}

CustomListItem* VirtualFileListView::AddFile(std::weak_ptr<struct AssetInfoBase> assetInfo)
{
	if (!assetInfo.expired())
	{
		//收集资产的ToolTip
		ToolTip newToolTip;
		for (auto& i : assetInfo.lock()->toolTips)
		{
			if (newToolTip._tooltip.length() > 1)
			{
				newToolTip._tooltip += "\n";
			}
			newToolTip._tooltip += i.c_str();
		}
		auto iconPath = assetInfo.lock()->absFilePath + ".png";
		if (!FileSystem::FileExist(iconPath))
		{
			iconPath = assetInfo.lock()->absFilePath + ".jpg";
		}
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
		return newItem;
	}
	return nullptr;
}
#pragma endregion

//--------------------------------------Repository Selection Widget-------------------
#pragma region RepositorySelectionWidget
RepositorySelection::RepositorySelection(QWidget* parent) :QWidget(parent)
{
	setObjectName("ContentBrowser_RepositorySelection");
	setAttribute(Qt::WidgetAttribute::WA_AlwaysStackOnTop);
	setWindowFlags(Qt::Window);
	setWindowFlags(Qt::Tool);
	setWindowFlag(Qt::FramelessWindowHint);
	setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(8, 8, 10, 8);
	setLayout(layout);
	adjustSize();
	combo = new ComboBox("RepositorySelection : ", this);
	layout->addWidget(combo);
	show();
	resize(0, 0);
	setHidden(true);
	_selectionCallBack = [](HString text, int index) {};
	combo->_bindCurrentTextChanged = [this](const int index, const char* text) 
	{
		setHidden(true);
		_selectionCallBack(text, index);
	};
}

void RepositorySelection::Show()
{
	show();
	resize(0, 0);
	setHidden(false);
	//
	combo->ClearItems();
	auto repositories = ContentManager::Get()->GetRepositories();
	for (auto& i : repositories)
	{
		HString name = i.first;
		combo->AddItem(name.c_str());
	}
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

ContentBrowser::ContentBrowser(QWidget* parent )
	:QWidget(parent)
{
	ui.setupUi(this);
	ui.horizontalLayout_2->setSpacing(1);
	ui.horizontalLayout_2->setContentsMargins(1, 1, 1, 1);
	ui.ContentBrowserVBoxLayout->setSpacing(1);
	ui.ContentBrowserVBoxLayout->setContentsMargins(1, 1, 1, 1);

	this->setObjectName("ContentBrowser");
	
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
	_treeView = new VirtualFolderTreeView(this);
	treeLayout->addWidget(_treeView);
	_listView = new VirtualFileListView(this);
	listLayout->addWidget(_listView);
	//
	_splitterBox->setStretchFactor(1, 4);
	ui.ContentBrowserVBoxLayout->addWidget(_splitterBox);
	ui.ContentBrowserVBoxLayout->setStretch(0,0);
	ui.ContentBrowserVBoxLayout->setStretch(1, 1000);
	
	//Path label
	ui.PathLabel->setObjectName("PathLabel");
	ui.PathLabel->setText("Asset -");

	//Repository Selection Widget
	{
		_repositorySelection = new RepositorySelection(this);
		_repositorySelection->_selectionCallBack = [this](HString text, int index)
		{
			//资产导入操作在这
			if (_importFileNames.size() > 0)
			{
				_currentRepositorySelection = text.c_str();
				for (auto& i : _importFileNames)
				{
					//QMessageBox::information(0,0,i,0);
					QFileInfo info(i);
					if (info.suffix().compare("fbx", Qt::CaseInsensitive) == 0)
					{

					}
					else if (info.suffix().compare("fbx", Qt::CaseInsensitive) == 0)
					{

					}
					else if (info.suffix().compare("png", Qt::CaseInsensitive) == 0
						|| info.suffix().compare("tga", Qt::CaseInsensitive) == 0
						|| info.suffix().compare("jpg", Qt::CaseInsensitive) == 0
						|| info.suffix().compare("bmp", Qt::CaseInsensitive) == 0
						|| info.suffix().compare("hdr", Qt::CaseInsensitive) == 0)
					{

					}
				}
				_importFileNames.clear();

			}
		};
	}

	_contentBrowser.append(this);
	//Connect
	connect(_treeView, &QTreeView::clicked, this, [this]() {
		_treeView->_bSaveSelectionItem = true;
		if (_treeView->_currentSelectionItem != 0 && _treeView->_newSelectionItems.size() > 0)
		{
			_treeView->_newSelectionItems.erase(_treeView->_newSelectionItems.begin(), _treeView->_newSelectionItems.end() - (_treeView->_newSelectionItems.size() - _treeView->_currentSelectionItem));
			_treeView->_currentSelectionItem = 0;
		}
	});
	connect(ui.ImportButton, &QPushButton::clicked, this, [this]() {
		// 创建一个支持多选的文件选择对话框
		QStringList fileNames = QFileDialog::getOpenFileNames(nullptr,
			"Import Resources",  // 对话框标题
			"",         // 对话框初始目录
			"All File (*);;\
				Model (*.fbx);;\
				Image (*.png *.jpg *.bmp *.tga *.hdr);;\
			");  
		if (fileNames.size() > 0)
		{
			_repositorySelection->Show();
			_importFileNames = fileNames;
		}	
	}); 
	connect(ui.FrontspaceButton, &QPushButton::clicked, this, [this]() {
		if (_treeView->_newSelectionItems.size() > 0 && _treeView->_currentSelectionItem > 0)
		{
			_treeView->_currentSelectionItem--;
			if (_treeView->_newSelectionItems.size() > _treeView->_currentSelectionItem)
			{
				auto item = _treeView->_newSelectionItems[_treeView->_currentSelectionItem];
				{
					_treeView->_bSaveSelectionItem = false;
					_treeView->selectionModel()->clearSelection();
					_treeView->selectionModel()->setCurrentIndex(item->index(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
				}
			}
		}
	});
	connect(ui.BackspaceButton, &QPushButton::clicked, this, [this]() {
		if (_treeView->_newSelectionItems.size() > 0)
		{
			_treeView->_currentSelectionItem++;
			if (_treeView->_newSelectionItems.size() > _treeView->_currentSelectionItem)
			{
				auto item = _treeView->_newSelectionItems[_treeView->_currentSelectionItem];
				{
					_treeView->_bSaveSelectionItem = false;
					_treeView->selectionModel()->clearSelection();
					_treeView->selectionModel()->setCurrentIndex(item->index(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
				}
			}
		}
	});

	connect(ui.BackToParentButton, &QPushButton::clicked, this, [this]() {
		if (_treeView->currentIndex().isValid())
		{
			_treeView->_bSaveSelectionItem = true;
			QStandardItemModel* model = (QStandardItemModel*)_treeView->model();
			CustomViewItem* item = (CustomViewItem*)model->itemFromIndex(_treeView->currentIndex());
			if (item->parent()->index().isValid())
			{
				_treeView->selectionModel()->clearSelection();
				_treeView->selectionModel()->setCurrentIndex(item->parent()->index(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
			}
		}
	});


	//Refresh
	RefreshContentBrowsers();
}

ContentBrowser::~ContentBrowser()
{
	_contentBrowser.removeOne(this);
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
	_treeView->RemoveAllItems();
	auto folders = ContentManager::Get()->GetVirtualFolders();
	for (auto& i : folders)
	{
		auto pathTag = i.second.Path.Split("/");
		CustomViewItem* parent = _treeView->_rootItem;
		for (int i = 0; i < pathTag.size(); i++)
		{
			QString vfp = parent->_path + "/" + parent->_text + "/" + pathTag[i].c_str() + "/";
			CustomViewItem* newItem = _treeView->FindFolder(vfp);
			if (newItem == nullptr)
			{
				newItem = new CustomViewItem(pathTag[i].c_str());
				_treeView->AddItem(newItem, parent);
			}
			parent = newItem;
		}
	}
}

void ContentBrowser::RefreshFileOnListView()
{
	_listView->RemoveAllItems();
	auto item = _treeView->GetSelectionItems();
	for (auto& i : item)
	{
		auto assets = ContentManager::Get()->GetAssetsByVirtualFolder(i->_fullPath.toStdString().c_str());
		for (auto& a : assets)
		{
			auto item = _listView->AddFile(a.second);
		}
	}
}

void ContentBrowser::focusInEvent(QFocusEvent* event)
{

}

void ContentBrowser::showEvent(QShowEvent* event)
{

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
}

void ContentBrowser::mousePressEvent(QMouseEvent* event)
{
}

void ContentBrowser::closeEvent(QCloseEvent* event)
{
}


#pragma endregion

