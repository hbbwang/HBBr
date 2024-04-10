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
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
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
#pragma region  ViewItem
CustomViewItem::CustomViewItem(const QString& text) :QStandardItem(text)
{
	setEditable(false);
}
CustomViewItem::CustomViewItem(const QIcon& icon, const QString& text) : QStandardItem(icon, text)
{
	setEditable(false);
}

CustomListItem::CustomListItem(const QString& text, QListWidget* view, int type):QListWidgetItem(text, view, type)
{

}

CustomListItem::CustomListItem(const QIcon& icon, const QString& text, QListWidget* view, int type) : QListWidgetItem(icon, text, view, type)
{

}

#pragma endregion

////////////-------------------------------------Custom Tree View----------------------------------------
#pragma region  CustomTreeView
CustomTreeView::CustomTreeView(QWidget* parent)
	:QTreeView(parent)
{
	QStandardItemModel* model = new QStandardItemModel(this);

	_rootItem = new CustomViewItem("root");

	model->appendRow(_rootItem);

	setModel(model);

	setHeaderHidden(true);

	//setRootIsDecorated(false);

	setObjectName("CustomTreeView");

}

void CustomTreeView::SetRootItemName(QString newText)
{
	_rootItem->setText(newText);
}

void CustomTreeView::AddItem(CustomViewItem* newItem, CustomViewItem* parent)
{
	if (!newItem)
	{
		return;
	}
	newItem->_text = parent->text();
	if (parent)
	{
		newItem->_path = parent->_path + "/" + parent->text();
		newItem->_fullPath = newItem->_path + "/" + newItem->_text;
		parent->appendRow(newItem);
	}
	else
	{
		newItem->_path = parent->_path + "/" + _rootItem->text();
		newItem->_fullPath = newItem->_path + "/" + newItem->_text;
		_rootItem->appendRow(newItem);
	}
	_allItems.append(newItem);
}

const QList<CustomViewItem*> CustomTreeView::GetSelectionItems() const
{
	QList<CustomViewItem*> result;
	auto indces = selectedIndexes();
	for (auto& i : indces)
	{
		auto item = ((QStandardItemModel*)model())->itemFromIndex(i);
		if (item)
		{
			result.append((CustomViewItem*)item);
		}
	}
	return result;
}

QList<CustomViewItem*> CustomTreeView::FindItems(QString name)
{
	QList<CustomViewItem*> result;
	for (auto& i : _allItems)
	{
		if (i->_path.compare(name, Qt::CaseInsensitive) == 0 )
		{
			result.append(i);
		}
	}
	return result;
}

void CustomTreeView::RemoveAllItems()
{
	_allItems.clear();
	model()->removeRows(_rootItem->row(), _rootItem->rowCount());
}
#pragma endregion


////////////-------------------------------------List View----------------------------------------
//自定义样式
class CustomListViewItemDelegate : public QStyledItemDelegate
{
public:
	QSize _iconSize;
	QSize _gridSize;
	QString _qssStyle;

	void UpdateStyle()
	{
		_qssStyle = GetSingleStyleFromFile("CustomListView");
	}

	CustomListViewItemDelegate(QObject* parent, QSize IconSize,  QSize GridSize ) : QStyledItemDelegate(parent)
	{
		_iconSize = IconSize;
		_gridSize = GridSize;
		UpdateStyle();
	}

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		QStyleOptionViewItem opt = option;
		initStyleOption(&opt, index);

		QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
		QString text = index.data(Qt::DisplayRole).toString();

		QRect rect = opt.rect;
		QFont font = opt.font;

		////高亮
		//bool bSelect = false;
		//for (auto& i : ((QListView*)parent())->selectionModel()->selectedIndexes())
		//{
		//	if (i.row() == index.row())
		//	{
		//		bSelect = true;
		//	}
		//}
		//if (bSelect)
		//{
		//	// 设置画刷颜色（矩形填充颜色）
		//	//QBrush brush(QColor(180,150,160));
		//	//painter->setBrush(brush);
		//	QPen pen(QColor(180, 150, 160));
		//	pen.setWidth(3);
		//	painter->setPen(pen);
		//	painter->drawRect(
		//		rect.x(),
		//		rect.y(),
		//		_gridSize.width(),
		//		_gridSize.height());
		//}
		//else
		//{
		//	painter->eraseRect(
		//		rect.x(),
		//		rect.y(),
		//		_gridSize.width(),
		//		_gridSize.height());
		//}

		QSize sub = _gridSize - _iconSize;

		// 绘制图标
		painter->drawPixmap(
			rect.x() + (sub.width()/2),
			rect.y() + (sub.height() / 2),
			_iconSize.width(), 
			_iconSize.height(), 
			icon.pixmap(_iconSize));

		QTextDocument textDoc;
		textDoc.setHtml(QString("<div style='%1'>%2</div>").arg(_qssStyle).arg(text));
		textDoc.setTextWidth(_gridSize.width());
		textDoc.setDefaultFont(font);

		QRect textRect = QRect(
			opt.rect.x() + (_gridSize.width() / 2) - (textDoc.size().width()/2),
			rect.y() + (sub.height() / 2) + _iconSize.height(),
			_gridSize.width(),
			textDoc.size().height()
		);

		painter->save();
		painter->translate(textRect.topLeft());
		textDoc.documentLayout()->draw(painter, QAbstractTextDocumentLayout::PaintContext());
		painter->restore();

		//// 使用QStyle绘制项的背景和焦点矩形，以便支持QSS
		//opt.rect = option.rect;
		//opt.text = QString(); // 清除原始文本，以免重复绘制
		//QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

		//// 使用QStyle绘制文本，以便支持QSS
		//QApplication::style()->drawItemText(painter, textRect, Qt::AlignLeft, opt.palette, opt.state & QStyle::State_Enabled, text);
	
	}
};

#pragma region CustomListView
CustomListView::CustomListView(QWidget* parent) :QListWidget(parent)
{
	setObjectName("CustomListView");
	
	setViewMode(ViewMode::IconMode);

	setLayoutMode(LayoutMode::Batched);//延迟

	setIconSize(QSize(100,100));//格子内的图标大小

	setGridSize(iconSize()*1.5f);//每个格子的大小

	setWrapping(true);//Item换行

	setUniformItemSizes(true);

	setSpacing(2);//间隔

	setItemAlignment(Qt::AlignLeft);

	setSortingEnabled(true);

	setMovement(QListView::Snap);//Item吸附

	setResizeMode(QListView::ResizeMode::Adjust);//自动缩放

	setTextElideMode(Qt::TextElideMode::ElideNone);//文本的省略模式

	setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);

	setDragEnabled(true);

	setAcceptDrops(true);

	setDefaultDropAction(Qt::MoveAction);

	//CustomListViewItemDelegate* itemDelegate = new CustomListViewItemDelegate(this,iconSize(),gridSize());
	//setItemDelegate(itemDelegate);

	//设置自动排序
	connect(this, SIGNAL(indexesMoved(const QModelIndexList&)), this, SLOT(indexesMoved(const QModelIndexList&)));
	
}

CustomListItem* CustomListView::AddItem(QString name, QString iconPath)
{
	CustomListItem* item = nullptr;
	if (!iconPath.isEmpty())
	{
		item= new CustomListItem(QIcon(iconPath),name,this);
	}
	else
	{
		item = new CustomListItem(name,this);
	}
	item->_iconPath = iconPath;
	addItem(item);
	_allItems.append(item);
	return item;
}

void CustomListView::RemoveAllItems()
{
	//model()->removeRows(0, model()->rowCount());
	//model()->removeColumns(0, model()->columnCount());
	for (auto& i : _allItems)
	{
		takeItem(row(i));
		delete i;
	}
	_allItems.clear();
}

void CustomListView::resizeEvent(QResizeEvent* e)
{
	QListWidget::resizeEvent(e);
	//((CustomListViewItemDelegate*)itemDelegate())->UpdateStyle();
}


void CustomListView::indexesMoved(const QModelIndexList& indexes)
{
}

#pragma endregion

