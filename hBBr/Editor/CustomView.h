#pragma once
#include <QTreeView>
#include <QListView>
#include <qlistwidget.h>
#include <QDir>
#include <QLabel>
#include <qtooltip.h>
#include <QStylePainter>
#include <QStyleOption>
#include <qstandarditemmodel.h>
#include "HString.h"
#include "Asset/ContentManager.h"

struct ToolTip
{
	QString _tooltip;
};

class ToolTipWidget : public QWidget
{
	Q_OBJECT
public:
	ToolTipWidget(QWidget*parent = nullptr) :QWidget(parent)
	{}
protected:
	virtual void paintEvent(QPaintEvent* event)override
	{
		QStylePainter painter(this);
		QStyleOption opt;
		opt.initFrom(this);
		opt.rect = rect();
		painter.drawPrimitive(QStyle::PE_Widget, opt);
		QWidget::paintEvent(event);
	}
};

class CustomViewItem :public QStandardItem
{
public:
	explicit CustomViewItem(const QString& text);
	CustomViewItem(const QIcon& icon, const QString& text);
	QString _text;//treeview backup
	QString _path;//without name
	QString _fullPath;//with name
};

class CustomListItem :public QListWidgetItem
{
public:
	explicit CustomListItem(const QString& text, QListWidget* view = nullptr, int type = Type);
	explicit CustomListItem(const QIcon& icon, const QString& text,
		QListWidget* view = nullptr, int type = Type);
	QString _path;//without name
	QString _fullPath;//with name
	QString _iconPath;
	ToolTip _toolTip;
	std::weak_ptr<AssetInfoBase> _assetInfo;
};

class CustomTreeView : public QTreeView
{
	Q_OBJECT
public:
	explicit CustomTreeView(QWidget* parent = nullptr);

	virtual void AddItem(CustomViewItem* newItem, CustomViewItem* parent = nullptr);

	const QList<CustomViewItem*> GetSelectionItems()const;

	QList<CustomViewItem*> FindItems(QString name);

	CustomViewItem* FindItem(QString path);

	virtual void SelectionItem(QString itemPath);

	virtual void RemoveAllItems();

	//移除符合名字的items
	virtual void RemoveItems(QString name);

	QList<CustomViewItem*> _allItems;
};

class CustomListView : public QListWidget
{
	Q_OBJECT
public:

	explicit CustomListView(QWidget* parent = nullptr);
	~CustomListView();

	virtual CustomListItem* AddItem(QString name, QString iconPath = "", ToolTip toolTip = ToolTip());

	virtual QList<CustomListItem*> FindItems(QString name);

	virtual CustomListItem* FindItem(QString itemPath);

	virtual const QList<CustomListItem*> GetSelectionItems()const;

	void RemoveAllItems();

	QList<CustomListItem*> _allItems;

protected:
	void resizeEvent(QResizeEvent* e) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	void leaveEvent(QEvent* event) override;

	CustomListItem* _currentMouseTrackItem;

	ToolTipWidget* _toolTipWidget;
	QLabel* _toolTipLabel = nullptr;
	//QLabel* _dirtyMark = nullptr;

private slots:
	void indexesMoved(const QModelIndexList& indexes);

};
