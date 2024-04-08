#pragma once

#include <QTreeView>
#include <QListView>
#include <QDir>
#include <qstandarditemmodel.h>
#include "HString.h"
#include "Asset/ContentManager.h"

class CustomViewItem :public QStandardItem
{
public:
	explicit CustomViewItem(const QString& text);
	QString path;
};

class CustomTreeView : public QTreeView
{
	//Q_OBJECT
public:
	explicit CustomTreeView(QWidget* parent = nullptr);
	
	void SetRootItemName(QString newText);

	void AddItem(CustomViewItem* newItem, CustomViewItem* parent = nullptr);

	QList<CustomViewItem*> FindItems(QString name);

	void RemoveAllItems();

	QStandardItemModel* _model = nullptr;

	CustomViewItem* _rootItem = nullptr;

	QList<CustomViewItem*> _allItems;
};

//class CustomListView : public QListView
//{
//	Q_OBJECT
//public:
//	explicit CustomListView(QWidget* parent = nullptr);
//
//};
