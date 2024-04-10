#pragma once
#include <QTreeView>
#include <QListView>
#include <qlistwidget.h>
#include <QDir>
#include <qstandarditemmodel.h>
#include "HString.h"
#include "Asset/ContentManager.h"

class CustomViewItem :public QStandardItem
{
public:
	explicit CustomViewItem(const QString& text);
	CustomViewItem(const QIcon& icon, const QString& text);
	std::weak_ptr<AssetInfoBase*> _assetInfo;
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
};

class CustomTreeView : public QTreeView
{
	Q_OBJECT
public:
	explicit CustomTreeView(QWidget* parent = nullptr);
	
	void SetRootItemName(QString newText);

	virtual void AddItem(CustomViewItem* newItem, CustomViewItem* parent = nullptr);

	const QList<CustomViewItem*> GetSelectionItems()const;

	QList<CustomViewItem*> FindItems(QString name);

	void RemoveAllItems();

	CustomViewItem* _rootItem = nullptr;

	QList<CustomViewItem*> _allItems;
};

class CustomListView : public QListWidget
{
	Q_OBJECT
public:

	explicit CustomListView(QWidget* parent = nullptr);

	virtual CustomListItem* AddItem(QString name, QString iconPath = "");

	void RemoveAllItems();

	QList<CustomListItem*> _allItems;

protected:
	void resizeEvent(QResizeEvent* e) override;

private slots:
	void indexesMoved(const QModelIndexList& indexes);

};
