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

VirtualFolderTreeView::VirtualFolderTreeView(QWidget* parent)
	:CustomTreeView(parent)
{
	setObjectName("CustomTreeView_VirtualFolderTreeView");
	_rootItem->setText("Asset");
	_rootItem->path = "";
	setIndentation(20);
	setEditTriggers(EditTrigger::DoubleClicked);
}

CustomViewItem* VirtualFolderTreeView::FindFolder(QString virtualPath)
{
	//auto folderName = FileSystem::GetFileName(virtualPath.toStdString().c_str());
	//FindItem
	for (auto& i : _allItems)
	{
		QString shortPath = i->path + i->text();
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
	_splitterBox->setObjectName("ContentBrowserSplitter");
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
	//
	_splitterBox->setStretchFactor(1, 4);
	ui.ContentBrowserVBoxLayout->addWidget(_splitterBox);
	ui.ContentBrowserVBoxLayout->setStretch(0,0);
	ui.ContentBrowserVBoxLayout->setStretch(1, 1000);
	
	//Path label
	ui.PathLabel->setObjectName("PathLabel");

	_contentBrowser.append(this);
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

void ContentBrowser::SpawnFolder(VirtualFolder& folder)
{
	auto pathTag = folder.Path.Split("/");
	CustomViewItem* parent = _treeView->_rootItem;
	for (int i = 0; i < pathTag.size();i++)
	{
		QString vfp = parent->path + "/" + parent->text() + "/" + pathTag[i].c_str() + "/";
		CustomViewItem* newItem = _treeView->FindFolder(vfp);
		if (newItem == nullptr)
		{
			newItem = new CustomViewItem(pathTag[i].c_str());
			_treeView->AddItem(newItem, parent);
		}
		parent = newItem;
	}
}

void ContentBrowser::Refresh()
{
	_treeView->RemoveAllItems();
	//auto folders = FileSystem::GetAllFolders(FileSystem::GetContentAbsPath().c_str());
	auto folders = ContentManager::Get()->GetVirtualFolders();

	for (auto& i : folders)
	{
		SpawnFolder(i.second);
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

