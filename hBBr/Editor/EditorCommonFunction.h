#pragma once
#include "QString.h"
#include <QFileInfo>

QString GetWidgetStyleSheetFromFile(QString objectName, QString path = "Config/Theme/ThemeMain.qss");

QString GetSingleStyleFromFile(QString Name, QString path = "Config/Theme/ThemeMain.qss");

#define ActionConnect(action,func) connect(action, &QAction::triggered, this, func);

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

QString GetEditorInternationalization(QString Group, QString name);

bool GetEditorInternationalizationInt(QString Group, QString name, int& result);

void SetEditorInternationalizationInt(QString Group, QString name, int newValue);

QString GetEditorConfig(QString Group, QString name);

void SetWindowCenterPos(QWidget* widget);

void SaveEditorWindowSetting(QWidget* widget, QString Group);

void LoadEditorWindowSetting(QWidget* widget, QString Group);