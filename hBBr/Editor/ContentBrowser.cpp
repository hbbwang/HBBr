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
#include "EditorMain.h"
#include "ConsoleDebug.h"
//--------------------------------------VirtualFolderTreeView-------------------
#pragma region VirtualFolderTreeView
VirtualFolderTreeView::VirtualFolderTreeView(class  ContentBrowser* contentBrowser, QWidget* parent)
	:CustomTreeView(parent)
{
	setObjectName("CustomTreeView_VirtualFolderTreeView");
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
	if (parent)
	{
		newItem->_path = parent->_path + "/" + parent->_text;
		newItem->_fullPath = newItem->_path + "/" + newItem->_text;
		parent->appendRow(newItem);
	}
	else
	{
		//newItem->_path = parent->_path + "/" + _rootItem->_text;
		newItem->_fullPath = newItem->_text;
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
	auto item = FindFolder(vPath);
	selectionModel()->clearSelection();
	selectionModel()->setCurrentIndex(item->index(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
}

void VirtualFolderTreeView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	_contentBrowser->RefreshFileOnListView();
	CustomTreeView::selectionChanged(selected, deselected);
}

void VirtualFolderTreeView::RemoveFolder(QString virtualPath)
{
	auto item = FindFolder(virtualPath);
	_allItems.removeOne(item);
	model()->removeRow(item->row());
	//尝试删除历史记录
	_newSelectionItems.removeOne(item);
	//删除实际路径文件和assetInfo
	
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
	setAttribute(Qt::WidgetAttribute::WA_AlwaysStackOnTop);
	setWindowFlags(Qt::Window);
	setWindowFlags(Qt::Tool);
	setWindowFlag(Qt::FramelessWindowHint);
	setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
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
			Hide();
		});
		connect(bCancel, &QPushButton::clicked, this, [this]() {
			Hide();
		});
	}
	//
	adjustSize();
	show();
	resize(0, 0);
	setHidden(true);
	combo->_bindCurrentTextChanged = [this](const int index, const char* text) 
	{
		_currentRepositorySelection = text;
	};
	combo->ui.horizontalLayout->setContentsMargins(1, 1, 1, 1);
	//combo->setMaximumHeight(50);
	combo->ui.ComboBox_0->setMaximumHeight(40);
	combo->ui.Name->setMaximumHeight(45);
}

void RepositorySelection::Show()
{
	show();
	resize(300, 0);
	QPoint pos = QPoint(EditorMain::_self->x() + EditorMain::_self->width() / 2, EditorMain::_self->y() + EditorMain::_self->height()/2);
	pos -= QPoint(width()/2 , height()/2 );
	//pos = EditorMain::_self->mapToGlobal(pos);

	setGeometry(pos.x(), pos.y(), 400 , 0);
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

void RepositorySelection::Hide()
{
	resize(0,0);
	move(0,0);
	setHidden(true);
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
RepositorySelection* ContentBrowser::_repositorySelection = nullptr;
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
		_repositorySelection->setObjectName("ContentBrowser_RepositorySelection");
		_repositorySelection->_selectionCallBack = [this](QString repository)
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
						newInfo.virtualPath =((CustomViewItem*)((QStandardItemModel*) _treeView->model())->itemFromIndex(_treeView->currentIndex()))->_fullPath.toStdString().c_str();
						infos.push_back(newInfo);
					}
				}
				bool bSucceed = ContentManager::Get()->AssetImport(repository.toStdString().c_str() , infos);
				_importFileNames.clear();
				if (bSucceed)
				{
					RefreshContentBrowsers();
				}
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
		ConsoleDebug::print_endl("Begin import assets...");
		if (fileNames.size() > 0)
		{
			_repositorySelection->setFocus();
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
			_treeView->_bSaveSelectionItem = false;
			_treeView->SelectionItem(currentItemVPath);
		}
		else
		{
			_treeView->_bSaveSelectionItem = false;
			_treeView->selectionModel()->clearSelection();
			_treeView->selectionModel()->setCurrentIndex(firstChoose->index(), QItemSelectionModel::SelectionFlag::ClearAndSelect);

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

