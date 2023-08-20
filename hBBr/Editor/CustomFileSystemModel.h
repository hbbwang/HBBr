#pragma once

#include <qfilesystemmodel.h>
#include "CustomFileIconProvider.h"
#include <QTreeView>
class CustomTreeView;

class CustomFileSystemModel  : public QFileSystemModel
{
	Q_OBJECT
public:
	CustomFileSystemModel(QObject *parent);
	~CustomFileSystemModel();

	CustomTreeView* _contentBrowserTreeView=NULL;

	QVariant data(const QModelIndex& index, int role) const;

};
