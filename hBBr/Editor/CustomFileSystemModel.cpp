#include "CustomFileSystemModel.h"
#include <qdebug.h>
#include <qpixmap.h>
#include "HString.h"
#include <QFileInfo>
#include "CustomFileSystemView.h"

#include "Resource/ContentManager.h"

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

	//改变文件显示的名字
	if (_contentBrowserListView)
	{
		QString path = this->filePath(index);
		QFileInfo info (path);
		if (info.exists() && info.isFile() && role == Qt::TextDate)
		{
			auto assetInfo = _contentBrowserListView->_fileInfos[index];
			if (assetInfo)
			{
				return  assetInfo->name.c_str();
			}
		}	
	}
	return __super::data(index, role);
}
