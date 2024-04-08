#include "CustomView.h"
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

////////////-------------------------------------View Item----------------------------------------

CustomViewItem::CustomViewItem(const QString& text) :QStandardItem(text)
{
	setEditable(false);
}


////////////-------------------------------------Tree View----------------------------------------

CustomTreeView::CustomTreeView(QWidget* parent)
	:QTreeView(parent)
{
	_model = new QStandardItemModel(this);

	_rootItem = new CustomViewItem("root");

	_model->appendRow(_rootItem);

	setHeaderHidden(true);

	//setRootIsDecorated(false);

	setObjectName("CustomTreeView");

	setModel(_model);
}

void CustomTreeView::SetRootItemName(QString newText)
{
	_rootItem->setText(newText);
}

void CustomTreeView::AddItem(CustomViewItem* newItem, CustomViewItem* parent)
{
	if (parent)
	{
		newItem->path = parent->path + "/" + parent->text();
		parent->appendRow(newItem);
	}
	else
	{
		newItem->path = parent->path + "/" + _rootItem->text();
		_rootItem->appendRow(newItem);
	}
	_allItems.append(newItem);
}

QList<CustomViewItem*> CustomTreeView::FindItems(QString name)
{
	QList<CustomViewItem*> result;
	for (auto& i : _allItems)
	{
		if (i->path.compare(name, Qt::CaseInsensitive) == 0 )
		{
			result.append(i);
		}
	}
	return result;
}

void CustomTreeView::RemoveAllItems()
{
	_model->removeRows(_rootItem->row(), _rootItem->rowCount());
}


////////////-------------------------------------List View----------------------------------------

//
//CustomListView::CustomListView(QWidget* parent) :QListView(parent)
//{
//
//}
