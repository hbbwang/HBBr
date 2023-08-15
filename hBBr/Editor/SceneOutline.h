#pragma once
#include <QWidget>
#include <qstandarditemmodel.h>
#include <qtreewidget.h>
#include <qlayout.h>
class GameObjectItem :public QTreeWidgetItem
{
public:
    GameObjectItem(class GameObject* gameObject, QTreeWidget* view);
    class GameObject* _gameObject = NULL;
    void Destroy();
};

class GameObjectItemModel :public QStandardItemModel
{
    Q_OBJECT
public:
    GameObjectItemModel(QObject* parent = nullptr) {}
};

class SceneOutlineTree :public QTreeWidget
{
public:
    explicit SceneOutlineTree(QWidget* parent = nullptr);
    ~SceneOutlineTree() {}

    GameObjectItemModel* _model; 
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

