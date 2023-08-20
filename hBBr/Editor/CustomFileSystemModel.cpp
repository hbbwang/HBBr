#include "CustomFileSystemModel.h"
#include <qdebug.h>
#include <qpixmap.h>
#include "HString.h"
#include <QFileInfo>
#include "CustomFileSystemView.h"
CustomFileSystemModel::CustomFileSystemModel(QObject *parent)
	: QFileSystemModel(parent)
{
}

CustomFileSystemModel::~CustomFileSystemModel()
{}

QVariant CustomFileSystemModel::data(const QModelIndex & index, int role) const
{

	//if (_contentBrowserTreeView)
	//{
	//	if (role == Qt::SizeHintRole)
	//	{
	//		return _contentBrowserTreeView->_iconSize;
	//	}
	//}

	return __super::data(index, role);
}
