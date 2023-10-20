#pragma once
#include "QString.h"
#include <QFileInfo>

QString GetWidgetStyleSheetFromFile(QString objectName, QString path = "Config/Theme/ThemeMain.qss");

bool DeleteAllFile(QString path, QList<QString>*allAssets = NULL);

struct SFileSearch
{
	QString path;
	QFileInfo fileInfo;
	bool bLastFile = false;
};
//只搜索目录
bool SearchDir(QString path, QString searchText ,QList<SFileSearch>& ResultOutput);
//搜索当前目录下的制定文件
bool SearchFile(QString path, QString searchText, QList<SFileSearch>& ResultOutput);
//获取TGA图像
QImage GetImageFromTGA(QString path);

bool GetPreviewImage(QString resourceFilePath , QPixmap& pixmap);
