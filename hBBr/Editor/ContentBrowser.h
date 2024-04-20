#pragma once
#include "CustomView.h"
#include "ui_ContentBrowser.h"
#include "Asset/ContentManager.h"
#include <qdialog.h>
//--------------------------------------Virtual Folder Tree View-------------------
#pragma region VirtualFolderTreeView
class VirtualFolderTreeView : public CustomTreeView
{
	Q_OBJECT
	friend class ContentBrowser;
	friend class VirtualFileListView;
public:
	explicit VirtualFolderTreeView(class  ContentBrowser* contentBrowser , QWidget* parent = nullptr);

	void AddItem(CustomViewItem* newItem, CustomViewItem* parent = nullptr)override;

	//删除文件(是真的删除!包括真实路径下的文件,注意别乱用了)
	//和RemoveItem不一样，RemoveItem只是暂时移除编辑器里的虚拟目录，不会对项目造成实质性影响。
	virtual void RemoveFolder(QString virtualPath);

	//虚拟路径统一用“/”分割，不存在使用"\\"
	CustomViewItem* FindFolder(QString virtualPath);

	virtual void SelectionItem(QString vPath)override;

	class  ContentBrowser* _contentBrowser;

protected:
	virtual void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)override;

	void currentChanged(const QModelIndex& current, const QModelIndex& previous) override;

	virtual void contextMenuEvent(QContextMenuEvent* event)override;

	class QMenu* _contextMenu = nullptr;

	QList<CustomViewItem*> _newSelectionItems;

	int _currentSelectionItem;

	bool _bSaveSelectionItem;

};
#pragma endregion

//--------------------------------------Virtual File List View-------------------
#pragma region VirtualFileListView
class VirtualFileListView :public CustomListView
{
	Q_OBJECT
	friend class ContentBrowser;
	friend class VirtualFolderTreeView;
public:
	explicit VirtualFileListView(class  ContentBrowser* contentBrowser, QWidget* parent = nullptr);

	CustomListItem* AddFile(std::weak_ptr<struct AssetInfoBase> assetInfo);

	VirtualFolder _currentTreeViewSelection;

	class  ContentBrowser* _contentBrowser;

protected:

	class QMenu* _contextMenu = nullptr;

	virtual void contextMenuEvent(QContextMenuEvent* event)override;

	//滚动到指定Item位置
	//void scrollToItem(const QListWidgetItem* item, QAbstractItemView::ScrollHint hint = EnsureVisible);

	//当前选中的虚拟路径的所有文件对象
	QList<CustomListItem*> _currentVirtualFolderItems;

private:
	CustomListItem* _ediingItem = nullptr;
private slots:
	void ItemTextChange(QListWidgetItem* item);

};
#pragma endregion

//--------------------------------------Repository Selection Widget-------------------
#pragma region RepositorySelectionWidget
class RepositorySelection : public QDialog
{
	Q_OBJECT
	friend class VirtualFileListView;
	friend class VirtualFolderTreeView;
	friend class ContentBrowser;
public:
	RepositorySelection(QWidget* parent);
	void Show();
	void Hide();
	std::function<void(QString repository)> _selectionCallBack;
	QString _currentRepositorySelection;
protected:
	virtual void showEvent(QShowEvent*) override;
	virtual void paintEvent(QPaintEvent* event)override;
	virtual void resizeEvent(QResizeEvent* event)override;
	class ComboBox* combo = nullptr;
};

#pragma endregion

//--------------------------------------Content Browser Widget-------------------
#pragma region ContentBrowserWidget
class ContentBrowser : public QWidget
{
	Q_OBJECT
	friend class VirtualFileListView;
	friend class VirtualFolderTreeView;
	friend class RepositorySelection;
public:

	ContentBrowser(QWidget* parent = nullptr);
	~ContentBrowser();

	static void RefreshContentBrowsers();
	void Refresh();
	void RefreshFolderOnTreeView();
	void RefreshFileOnListView();

	static class RepositorySelection* _repositorySelection;

protected:

	virtual void focusInEvent(QFocusEvent* event);
	virtual void showEvent(QShowEvent* event);
	virtual void paintEvent(QPaintEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void closeEvent(QCloseEvent* event) override;
	static QList<ContentBrowser*> _contentBrowser;

	class QSplitter*		_splitterBox = nullptr;
	class QWidget*		_listWidget = nullptr;
	class QWidget*		_treeWidget = nullptr;



	VirtualFolderTreeView*		_treeView = nullptr;
	VirtualFileListView*			_listView = nullptr;

	QStringList _importFileNames;

private:

	Ui::ContentBrowserClass ui;

};
#pragma endregion