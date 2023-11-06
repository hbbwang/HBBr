#pragma once
#include <QWidget>
#include <qstandarditemmodel.h>
#include <qtreewidget.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <QMimeData>
#include <qlineedit.h>

class GameObjectItem :public QTreeWidgetItem
{
public:
    GameObjectItem(class GameObject* gameObject, QTreeWidget* view);
    class GameObject* _gameObject = NULL;
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
    VulkanRenderer* _renderer = NULL;
    QTreeWidgetItem* _curItem;
    QTreeWidgetItem* _mouseTouchItem;

    QMenu*      _menu = NULL;
    QAction*    _createNewGameObject = NULL;
    QAction*    _renameGameObject = NULL;
    QAction*    _deleteGameObject = NULL;

    QMenu* _menu_createBasic = NULL;
    QAction* _createCube = NULL;
    QAction* _createSphere = NULL;
    QAction* _createPlane = NULL;

    virtual void contextMenuEvent(QContextMenuEvent*) override;

private slots:

    void ItemDoubleClicked(QTreeWidgetItem* item, int column);
    void ItemEditFinished(QString newText);

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
    class  VulkanRenderer* _renderer = NULL;
    static SceneOutlineTree* _treeWidget;
    class CustomSearchLine* _search;

private slots:
    void TreeSearch();

};

