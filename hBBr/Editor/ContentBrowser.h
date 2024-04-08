#pragma once
#include "CustomView.h"
#include "ui_ContentBrowser.h"
#include "Asset/ContentManager.h"

//重做...
class VirtualFolderTreeView : public CustomTreeView
{
	Q_OBJECT
public:
	explicit VirtualFolderTreeView(QWidget* parent = nullptr);
	//虚拟路径统一用“/”分割，不存在使用"\\"
	CustomViewItem* FindFolder(QString virtualPath);
};

class ContentBrowser : public QWidget
{
	Q_OBJECT

public:
	ContentBrowser(QWidget* parent = nullptr);
	~ContentBrowser();

	static void RefreshContentBrowsers();
	void Refresh();
	void SpawnFolder(VirtualFolder& folder);
protected:
	virtual void focusInEvent(QFocusEvent* event);
	virtual void showEvent(QShowEvent* event);
	virtual void paintEvent(QPaintEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void closeEvent(QCloseEvent* event) override;
	static QList<ContentBrowser*> _contentBrowser;

	class QSplitter*					_splitterBox = nullptr;
	class QWidget*					_listWidget = nullptr;
	class QWidget*					_treeWidget = nullptr;

	VirtualFolderTreeView* _treeView = nullptr;

private:
	Ui::ContentBrowserClass ui;
};
