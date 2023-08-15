#pragma once
#include <QWidget>
#include <qstandarditemmodel.h>
#include <qtreewidget.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <QMimeData>


class GameObjectItem :public QTreeWidgetItem
{
public:
    GameObjectItem(class GameObject* gameObject, QTreeWidget* view);
    class GameObject* _gameObject = NULL;
    void Destroy();
};

class TreeItemMimeData : public QMimeData
{
    Q_OBJECT
public:
    GameObjectItem* _item = NULL;
};

class SceneOutlineTree :public QTreeWidget
{
    Q_OBJECT
public:
    explicit SceneOutlineTree(QWidget* parent = nullptr);
    ~SceneOutlineTree() {}
    virtual void mousePressEvent(QMouseEvent* event)override ;
    virtual void mouseMoveEvent(QMouseEvent* event)override;
    virtual void dropEvent(class QDropEvent* event) override;
protected:

    virtual void commitData(QWidget* editor)
    {
        QString newText = ((QLineEdit*)editor)->text();    // 获取编辑完成后的内容
        emit sigEditFinished(newText);    // 编辑完成后发出信号

        QTreeWidget::commitData(editor);
    }

    QTreeWidgetItem* _curItem;
    QTreeWidgetItem* _mouseTouchItem;
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

    SceneOutlineTree* _treeWidget = NULL;
};

