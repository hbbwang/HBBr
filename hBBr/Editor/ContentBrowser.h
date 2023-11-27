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
class ContentBrowser : public QWidget
{
	Q_OBJECT

public:
	ContentBrowser(QWidget *parent = nullptr);
	~ContentBrowser();

	class QSplitter*		_splitterBox = NULL;
	class CustomListView*	_listWidget = NULL;
	class CustomTreeView*	_treeWidget = NULL;

	QWidget* _tree_group = NULL;
	QWidget* _list_group = NULL;

	CustomSearchLine* _treeSearchLine = NULL;
	CustomSearchLine* _listSearchLine = NULL;

	CustomFileIconProvider		_treeFileIconProvider;
	CustomFileIconProvider		_listFileIconProvider;

	CustomFileSystemModel*		_treeFileSystemModel = NULL;
	CustomFileSystemModel*		_listFileSystemModel = NULL;

	QStringList list_filterCache;

	QStringList list_nameFilter_line;
	QStringList list_nameFilter_combo;

	QDir::Filters list_filters;

	MainWindow* _mainWindow = NULL;

	static void SearchAssetFile(HString filePath , ContentBrowser* cb = NULL);

	static QWidget* _currentFocusContentBrowser;

	static QList<QWidget*> _allContentBrowser;

protected:
	virtual void focusInEvent(QFocusEvent* event);
	virtual void showEvent(QShowEvent* event);
	virtual void paintEvent(QPaintEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void closeEvent(QCloseEvent* event) override;
private:
	Ui::ContentBrowserClass ui;

private slots:
//  void clicked(const QModelIndex &index);
	void TreeClicked(const QModelIndex& index);
	void ListClicked(const QModelIndex& index);
	void ListDoubleClicked(const QModelIndex& index);
	void CreateFolder();
	void TreeSearch();
	void ListSearch();
	void ListFilter(const QString& newText);
	void Backspace();
	void AssetImport();
	void OpenCurrentFolder_List();
	void OpenCurrentFolder_Tree();
};
