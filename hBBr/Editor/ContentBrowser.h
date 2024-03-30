#pragma once

#include <QWidget>
#include "ui_ContentBrowser.h"
#include "CustomFileIconProvider.h"
#include <QDir>
#include "HString.h"
class QSplitter;
class QListView;
class QTreeView;
class CustomFileSystemModel;
class CustomListView;
class CustomTreeView;
class CustomSearchLine;
class MainWindow;
//class ContentBrowser : public QWidget
//{
//	Q_OBJECT
//
//public:
//	ContentBrowser(QWidget *parent = nullptr);
//	~ContentBrowser();
//
//	class QSplitter*		_splitterBox = nullptr;
//	class CustomListView*	_listWidget = nullptr;
//	class CustomTreeView*	_treeWidget = nullptr;
//
//	QWidget* _tree_group = nullptr;
//	QWidget* _list_group = nullptr;
//
//	CustomSearchLine* _treeSearchLine = nullptr;
//	CustomSearchLine* _listSearchLine = nullptr;
//
//	CustomFileIconProvider		_treeFileIconProvider;
//	CustomFileIconProvider		_listFileIconProvider;
//
//	CustomFileSystemModel*		_treeFileSystemModel = nullptr;
//	CustomFileSystemModel*		_listFileSystemModel = nullptr;
//
//	QStringList list_filterCache;
//
//	QStringList list_nameFilter_line;
//	QStringList list_nameFilter_combo;
//
//	QDir::Filters list_filters;
//
//	MainWindow* _mainWindow = nullptr;
//
//	static void SearchAssetFile(HString filePath , ContentBrowser* cb = nullptr);
//
//	static QWidget* _currentFocusContentBrowser;
//
//	static QList<QWidget*> _allContentBrowser;
//
//protected:
//	virtual void focusInEvent(QFocusEvent* event);
//	virtual void showEvent(QShowEvent* event);
//	virtual void paintEvent(QPaintEvent* event);
//	virtual void mouseMoveEvent(QMouseEvent* event) override;
//	virtual void mousePressEvent(QMouseEvent* event);
//	virtual void closeEvent(QCloseEvent* event) override;
//private:
//	Ui::ContentBrowserClass ui;
//
//private slots:
////  void clicked(const QModelIndex &index);
//	void TreeClicked(const QModelIndex& index);
//	void ListClicked(const QModelIndex& index);
//	void ListDoubleClicked(const QModelIndex& index);
//	void CreateFolder();
//	void TreeSearch();
//	void ListSearch();
//	void ListFilter(const QString& newText);
//	void Backspace();
//	void AssetImport();
//	void OpenCurrentFolder_List();
//	void OpenCurrentFolder_Tree();
//};


//重做...
class ContentBrowser : public QWidget
{
	Q_OBJECT

public:
	ContentBrowser(QWidget* parent = nullptr);
	~ContentBrowser();

protected:
	virtual void focusInEvent(QFocusEvent* event);
	virtual void showEvent(QShowEvent* event);
	virtual void paintEvent(QPaintEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void closeEvent(QCloseEvent* event) override;

	class QSplitter*					_splitterBox = nullptr;
	class QWidget*					_listWidget = nullptr;
	class QWidget*					_treeWidget = nullptr;

private:
	Ui::ContentBrowserClass ui;
};
