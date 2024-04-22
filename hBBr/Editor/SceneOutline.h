#pragma once
#include <QWidget>
#include <qstandarditemmodel.h>
#include <qtreewidget.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <QMimeData>
#include <qlineedit.h>
#include "HGuid.h"
class GameObjectItem :public QTreeWidgetItem
{
public:
    GameObjectItem(class GameObject* gameObject, QTreeWidget* view);
    ~GameObjectItem();
    class GameObject* _gameObject = nullptr;
    void Destroy();
};

class SceneOutlineTree :public QTreeWidget
{
    Q_OBJECT
public:
    explicit SceneOutlineTree(class VulkanRenderer* renderer, QWidget* parent = nullptr);
    ~SceneOutlineTree() {}
    virtual void mousePressEvent(QMouseEvent* event)override ;
    virtual void mouseMoveEvent(QMouseEvent* event)override;
    virtual void dropEvent(class QDropEvent* event) override;
    QList<class GameObject*> GetSelectionObjects();
protected:

    virtual void commitData(QWidget* editor)
    {
        QString newText = ((QLineEdit*)editor)->text();    // 获取编辑完成后的内容
        emit sigEditFinished(newText);    // 编辑完成后发出信号

        QTreeWidget::commitData(editor);
    }
    VulkanRenderer* _renderer = nullptr;
    QTreeWidgetItem* _curItem;
    QTreeWidgetItem* _mouseTouchItem;

    QMenu*      _menu = nullptr;
    QAction*    _createNewGameObject = nullptr;
    QAction*    _renameGameObject = nullptr;
    QAction*    _deleteGameObject = nullptr;

    QMenu* _menu_createBasic = nullptr;
    QAction* _createCube = nullptr;
    QAction* _createSphere = nullptr;
    QAction* _createPlane = nullptr;

    virtual void contextMenuEvent(QContextMenuEvent*) override;

private slots:

    void ItemDoubleClicked(QTreeWidgetItem* item, int column);
    void ItemEditFinished(QString newText);
    void ItemSelectionChanged();
signals:
    void sigEditFinished(QString newText);
};

class SceneOutline : public QWidget
{
    Q_OBJECT
public:
    SceneOutline(class VulkanRenderer* renderer , QWidget *parent = nullptr);
    ~SceneOutline();

    virtual void closeEvent(QCloseEvent* event);
    virtual void focusInEvent(QFocusEvent* event);
    virtual void paintEvent(QPaintEvent* event);
    class  VulkanRenderer* _renderer = nullptr;
    class ComboBox* _currentLevel;
    static SceneOutlineTree* _treeWidget;
    class CustomSearchLine* _search;
private slots:
    void TreeSearch();

};

