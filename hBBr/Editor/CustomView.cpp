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
#include <QApplication>
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
	setFlags(Qt::ItemIsSelectable
		| Qt::ItemIsEditable
		| Qt::ItemIsDragEnabled
		| Qt::ItemIsDropEnabled
		| Qt::ItemIsEnabled
	);
}
CustomViewItem::CustomViewItem(const QIcon& icon, const QString& text) : QStandardItem(icon, text)
{
	setFlags(Qt::ItemIsSelectable
		| Qt::ItemIsEditable
		| Qt::ItemIsDragEnabled
		| Qt::ItemIsDropEnabled
		| Qt::ItemIsEnabled
	);
}

CustomListItem::CustomListItem(const QString& text, QListWidget* view, int type):QListWidgetItem(text, view, type)
{
	setFlags(Qt::ItemIsSelectable 
		| Qt::ItemIsEditable
		| Qt::ItemIsDragEnabled
		| Qt::ItemIsDropEnabled
		| Qt::ItemNeverHasChildren
		| Qt::ItemIsEnabled
	);
}

CustomListItem::CustomListItem(const QIcon& icon, const QString& text, QListWidget* view, int type) : QListWidgetItem(icon, text, view, type)
{
	setFlags(Qt::ItemIsSelectable
		| Qt::ItemIsEditable
		| Qt::ItemIsDragEnabled
		| Qt::ItemIsDropEnabled
		| Qt::ItemNeverHasChildren
		| Qt::ItemIsEnabled
	);
}

#pragma endregion

////////////-------------------------------------Custom Tree View----------------------------------------
#pragma region  CustomTreeView
CustomTreeView::CustomTreeView(QWidget* parent)
	:QTreeView(parent)
{
	QStandardItemModel* model = new QStandardItemModel(this);

	setModel(model);

	setHeaderHidden(true);

	//setRootIsDecorated(false);

	setObjectName("CustomTreeView");

	setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

	setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);

	setDragEnabled(true); 

	setAcceptDrops(true); 

	setDefaultDropAction(Qt::MoveAction);

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
		newItem->_path = "";
		newItem->_fullPath = newItem->_path + "/" + newItem->_text;
		((QStandardItemModel*)model())->appendRow(newItem);
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
		if (i->text().compare(name, Qt::CaseInsensitive) == 0 )
		{
			result.append(i);
		}
	}
	return result;
}

void CustomTreeView::SelectionItem(QString text)
{
}

void CustomTreeView::RemoveAllItems()
{
	_allItems.clear();
	model()->removeRows(0 , model()->rowCount());
}

void CustomTreeView::RemoveItems(QString name)
{
	auto items =FindItems(name);
	for (auto& i : items)
	{
		_allItems.removeOne(i);
		model()->removeRow(i->row());
	}
}

#pragma endregion


////////////-------------------------------------List View----------------------------------------
//自定义样式
class CustomListViewItemDelegate : public QStyledItemDelegate
{
public:
	CustomListView* _view;

	CustomListViewItemDelegate(QObject* parent) : QStyledItemDelegate(parent)
	{
	}

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
	{
		QStyleOptionViewItem opt = option;
		initStyleOption(&opt, index);
		QStyledItemDelegate::paint(painter, opt, index);

		auto baseRect = opt.rect;
		CustomListItem* item = (CustomListItem*)(_view->item(index.row()));
		//
		if (_view->viewMode() == QListWidget::IconMode &&  item && !item->_assetInfo.expired() && item->_assetInfo.lock()->bDirty)
		{
			// 绘制Dirty图标
			auto dirtyIconRect = baseRect;
			dirtyIconRect.setWidth(dirtyIconRect.width()/5);
			dirtyIconRect.setHeight(dirtyIconRect.height() / 5.5);
			dirtyIconRect.setX(dirtyIconRect.x() + 6);
			dirtyIconRect.setY(dirtyIconRect.y() + 6);
			HString pp = FileSystem::GetProgramPath() + "Config/Theme/Icons/ContentBrowser_AssetDirty.png";
			//auto icon = QIcon(pp.c_str());
			//QPixmap pixMap = icon.pixmap(dirtyIconRect.width() / 3, dirtyIconRect.width() / 3);
			QPixmap pixMap = QPixmap(pp.c_str());
			painter->drawPixmap(dirtyIconRect, pixMap);
		}
		//
	}
};

#pragma region CustomListView

CustomListView::CustomListView(QWidget* parent) :QListWidget(parent)
{
	setObjectName("CustomListView");
	
	setViewMode(ViewMode::IconMode);

	setLayoutMode(LayoutMode::Batched);//延迟

	setSelectionMode(QAbstractItemView::ExtendedSelection);//多选

	setIconSize(QSize(125,125));//格子内的图标大小

	setGridSize(QSize(iconSize().width()*1.15, iconSize().height()*1.3));//每个格子的大小

	setWrapping(true);//Item换行

	setWordWrap(true);

	setUniformItemSizes(true);

	//setSpacing(2);//间隔

	setItemAlignment(Qt::AlignLeft);

	setSortingEnabled(true);

	setMovement(QListView::Snap);//Item吸附

	setResizeMode(QListView::ResizeMode::Adjust);//自动缩放

	//setTextElideMode(Qt::TextElideMode::ElideNone);//文本的省略模式

	setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);

	setDragEnabled(true); 

	setAcceptDrops(true); 

	setDefaultDropAction(Qt::MoveAction);

	CustomListViewItemDelegate* itemDelegate = new CustomListViewItemDelegate(this);
	itemDelegate->_view = this;
	setItemDelegate(itemDelegate);

	{
		_toolTipWidget = new ToolTipWidget(this);
		_toolTipWidget->setObjectName("CustomListView_ToolTip");
		_toolTipWidget->setAttribute(Qt::WidgetAttribute::WA_AlwaysStackOnTop);
		_toolTipWidget->setWindowFlags(Qt::Window);
		_toolTipWidget->setWindowFlags(Qt::Tool);
		_toolTipWidget->setWindowFlag(Qt::FramelessWindowHint);
		_toolTipWidget->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
		_toolTipWidget->setFocusPolicy(Qt::FocusPolicy::NoFocus);
		_toolTipWidget->show();
		_toolTipWidget->resize(0,0);
		_toolTipWidget->setHidden(true);
		QVBoxLayout* layout = new QVBoxLayout(_toolTipWidget);
		layout->setContentsMargins(8, 8, 10, 8);
		_toolTipWidget->setLayout(layout);
		_toolTipWidget->adjustSize();
		_toolTipLabel = new QLabel(_toolTipWidget);
		_toolTipLabel->setObjectName("CustomListView_ToolTip_Text");
		//_toolTipLabel->setWordWrap(true);
		layout->addWidget(_toolTipLabel);
	}

	//设置自动排序
	connect(this, SIGNAL(indexesMoved(const QModelIndexList&)), this, SLOT(indexesMoved(const QModelIndexList&)));
	
}

CustomListView::~CustomListView()
{
}

CustomListItem* CustomListView::AddItem(QString name, QString iconPath, ToolTip toolTip)
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
	item->_toolTip = toolTip;
	item->_iconPath = iconPath;
	addItem(item);
	_allItems.append(item);
	return item;
}

QList<CustomListItem*> CustomListView::FindItems(QString name)
{
	QList<CustomListItem*> result;
	for (auto& i : _allItems)
	{
		if (i->_path.compare(name, Qt::CaseInsensitive) == 0)
		{
			result.append(i);
		}
	}
	return result;
}

const QList<CustomListItem*> CustomListView::GetSelectionItems() const
{
	{
		QList<CustomListItem*> result;
		auto indces = selectedIndexes();
		for (auto& i : indces)
		{
			auto item = itemFromIndex(i);
			if (item)
			{
				result.append((CustomListItem*)item);
			}
		}
		return result;
	}
}

void CustomListView::RemoveAllItems()
{
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
}

void CustomListView::mouseMoveEvent(QMouseEvent* event)
{
	QPoint pos = mapFromGlobal(event->globalPos());
	CustomListItem* item = (CustomListItem*)this->itemAt(pos);
	if (item && item->_toolTip._tooltip.length()>1)
	{
		if (_currentMouseTrackItem != item)
		{
			_currentMouseTrackItem = item;
		}
		_toolTipWidget->resize(0, 0);
		_toolTipWidget->setHidden(false);
		_toolTipLabel->setText(item->_toolTip._tooltip);
		_toolTipWidget->move(event->globalPos().x() + 10, event->globalPos().y() +10);
	}
	else
	{
		_toolTipWidget->setHidden(true);
		_currentMouseTrackItem = nullptr;
	}
	QListWidget::mouseMoveEvent(event);
}

void CustomListView::leaveEvent(QEvent* event)
{
	_toolTipWidget->setHidden(true);
	_currentMouseTrackItem = nullptr;
	QListWidget::leaveEvent(event);
}

void CustomListView::indexesMoved(const QModelIndexList& indexes)
{
	sortItems();
}

#pragma endregion

