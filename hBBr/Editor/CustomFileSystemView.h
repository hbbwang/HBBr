#pragma once

#include <QListView>
#include <qtreeview.h>
#include <qtooltip.h>

class QWidgetAction;
class QLineEdit;

class CustomListView  : public QListView
{
	Q_OBJECT

public:
	explicit CustomListView(QWidget *parent);
	~CustomListView();

	
	QAction* _import = NULL;
	QAction* _createNewFolder = NULL;
	QAction* _rename = NULL;
	QAction* _delete = NULL;
	QAction* _createMaterialInstance = NULL;
	QAction* _openCurrentFolder = NULL;
	QSize _iconSize = QSize(85,85);
	QSize _minIconSize = QSize(30, 30);
	QSize _maxIconSize = QSize(250, 250);

protected:
	bool bShowToolTip = false;
	QModelIndex _toolTipIndex;

	bool bCanResizeIcon = false;
	virtual void mousePressEvent(QMouseEvent* event)override;
	virtual void wheelEvent(QWheelEvent* event)override;
	virtual void mouseReleaseEvent(QMouseEvent* event)override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event)override;
	virtual void keyPressEvent(QKeyEvent* event)override;
	virtual void keyReleaseEvent(QKeyEvent* event)override;
	virtual void focusInEvent(QFocusEvent* event)override;
	virtual void focusOutEvent(QFocusEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	void dropEvent(QDropEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void showEvent(QShowEvent* event)override;
private slots:
	void CustomContextMenu(const QPoint& point);//创建右键菜单的槽函数
	void SlotClicked(const QModelIndex& index);
	void SlotPressed(const QModelIndex& index);
	void DeleteFile();
	void RenameFile();
};


class CustomTreeView : public QTreeView
{
	Q_OBJECT
public:
	explicit CustomTreeView(QWidget* parent);
	~CustomTreeView();

	QMenu*		_menu = NULL;
	QAction*	_import  = NULL;
	QAction*	_createNewFolder = NULL;
	QAction*	_rename = NULL;
	QAction*	_delete = NULL;
	QAction*	_openCurrentFolder = NULL;

	QSize _iconSize = QSize(40, 40);
	QSize _minIconSize = QSize(30, 30);
	QSize _maxIconSize = QSize(250, 250);
protected:
	bool bCanResizeIcon = false;
	virtual void mousePressEvent(QMouseEvent* event)override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event)override;
	virtual void wheelEvent(QWheelEvent* event)override;
	void dropEvent(QDropEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;

	virtual void keyPressEvent(QKeyEvent* event)override;
	virtual void keyReleaseEvent(QKeyEvent* event)override;
	virtual void focusOutEvent(QFocusEvent* event) override;

	QSize sizeHint(const QStyleOptionViewItem& option,
		const QModelIndex& index) const;

private slots:
	void CustomContextMenu(const QPoint& point);//创建右键菜单的槽函数
	void DeleteFile();
	void RenameFile();
};
