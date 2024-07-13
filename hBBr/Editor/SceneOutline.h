#pragma once
#include <QWidget>
#include <qstandarditemmodel.h>
#include <qtreewidget.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <QMimeData>
#include <qlineedit.h>
#include "HGuid.h"
#include "World.h"
#include <EditorMain.h>

class SceneOutlineItem :public QTreeWidgetItem
{
public:
    SceneOutlineItem(std::weak_ptr<Level> level, std::weak_ptr<GameObject> gameObject, QTreeWidget* view);
    SceneOutlineItem(std::weak_ptr<Level> level, std::weak_ptr<GameObject> gameObject, QString iconPath, QTreeWidget* view);
    SceneOutlineItem(std::weak_ptr<Level> level, std::weak_ptr<GameObject> gameObject, SceneOutlineItem* parent);
    ~SceneOutlineItem();
    void Init(std::weak_ptr<Level> level, std::weak_ptr<GameObject> gameObject , class SceneOutlineTree* tree);
    QString _iconPath;
    std::weak_ptr<GameObject> _gameObject;
    std::weak_ptr<Level> _level;
    void Destroy();
    SceneOutlineTree* _tree = nullptr;
protected:
    virtual QVariant data(int column, int role) const override;
};

class SceneOutlineTree :public QTreeWidget
{
    Q_OBJECT
        friend class SceneOutlineTreeDelegate;
public:

    explicit SceneOutlineTree(QWidget* parent = nullptr);
    ~SceneOutlineTree() {}
    virtual void mousePressEvent(QMouseEvent* event)override ;
    virtual void mouseMoveEvent(QMouseEvent* event)override;
    virtual void dragEnterEvent(class QDragEnterEvent* event) override;
    virtual void dropEvent(class QDropEvent* event) override;
    QList<std::weak_ptr<GameObject>> GetSelectionObjects();
    QList<std::weak_ptr<Level>> GetSelectionLevels();
    class SceneOutline* _parent = nullptr;
    SceneOutlineItem* _mouse_select_item = nullptr;
    SceneOutlineItem* IndexToItem(QModelIndex index);
    QList<SceneOutlineItem*> _allItems;
protected:

    virtual void commitData(QWidget* editor)
    {
        QString newText = ((QLineEdit*)editor)->text();    // ��ȡ�༭��ɺ������
        emit sigEditFinished(newText);    // �༭��ɺ󷢳��ź�

        QTreeWidget::commitData(editor);
    }
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

    QAction* _createNewLevel = nullptr;
    QAction* _deleteLevel = nullptr;
    QAction* _renameLevel = nullptr;
    QAction* _loadLevel = nullptr;
    QAction* _unloadLevel = nullptr;

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
    void ClearCurrentLevelSelection();
    void SetCurrentLevelSelection(SceneOutlineItem* item);
    void ResetLevelSelectionComboBox();
    class  VulkanRenderer* _renderer = nullptr;
    static SceneOutlineTree* _treeWidget;
    class CustomSearchLine* _search;
    class ComboBox* _comboBox;
    std::weak_ptr<World> currentWorld;

    SceneOutlineItem* _currentLevelItem = nullptr;
    SceneOutlineItem* FindLevel(QString levelName);
    SceneOutlineItem* FindLevel(HGUID guid);
    QMap<QString, SceneOutlineItem*>_levelItems;

    QSize sizeHint() const
    {
        return QSize(EditorMain::_self->width() / 5, EditorMain::_self->height() / 2);//���ﶨ��dockwidget�ĳ�ʼ��С
    }
private slots:
   // void TreeSearch();
};

