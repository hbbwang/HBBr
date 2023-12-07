#pragma once

#include <qfilesystemmodel.h>
#include "CustomFileIconProvider.h"
#include <QTreeView>

class CustomListView;
class CustomTreeView;

class CustomFileSystemModel  : public QFileSystemModel
{
	Q_OBJECT
public:
	CustomFileSystemModel(QObject *parent);
	~CustomFileSystemModel();

	CustomTreeView* _contentBrowserTreeView= nullptr;

	CustomListView* _contentBrowserListView = nullptr;

	QVariant data(const QModelIndex& index, int role) const;

};
