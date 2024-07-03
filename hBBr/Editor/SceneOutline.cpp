#include "SceneOutline.h"
#include "VulkanRenderer.h"
#include "Asset/World.h"
#include "Asset/Level.h"
#include "Component/GameObject.h"
#include <qheaderview.h>
#include <qaction.h>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QApplication>
#include <QDrag>
#include <QDir>
#include <QStyledItemDelegate>
#include <QStyleOption>
#include <QPainter>
#include <qmenu.h>
#include "EditorCommonFunction.h"
#include "CustomSearchLine.h"
#include "Inspector.h"
#include "ComboBox.h"
#include "FormMain.h"
#include "ConsoleDebug.h"
#include "DirtyAssetsManager.h"
SceneOutlineTree* SceneOutline::_treeWidget = nullptr;

SceneOutlineItem::SceneOutlineItem(std::weak_ptr<Level> level, std::weak_ptr<GameObject> gameObject, QTreeWidget* view)
    :QTreeWidgetItem(view)
{
    Init(level, gameObject, (SceneOutlineTree*)view);
}

SceneOutlineItem::SceneOutlineItem(std::weak_ptr<Level> level, std::weak_ptr<GameObject> gameObject, QString iconPath, QTreeWidget* view)
{
    Init(level, gameObject, (SceneOutlineTree*)view);
    QFileInfo info(iconPath);
    if (info.exists())
    {
        iconPath = QDir::toNativeSeparators(iconPath);
        _iconPath = iconPath;
    }
}

SceneOutlineItem::SceneOutlineItem(std::weak_ptr<Level> level, std::weak_ptr<GameObject> gameObject, SceneOutlineItem* parent)
    :QTreeWidgetItem(parent)
{
    Init(level, gameObject, parent->_tree);
}

SceneOutlineItem::~SceneOutlineItem()
{

}

void SceneOutlineItem::Init(std::weak_ptr<Level> level, std::weak_ptr<GameObject> gameObject, SceneOutlineTree* tree)
{
    if (!level.expired())
    {
        _level = level;
        this->setText(0, (_level.lock()->GetLevelName()).c_str());
        setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled);
        this->setData(0, Qt::UserRole, "SceneOutline_LevelItem");
    }
    else if (!gameObject.expired())
    {
        _gameObject = gameObject;
        _gameObject.lock()->_editorObject = this;
        this->setText(0, _gameObject.lock()->GetObjectName().c_str());
        setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
    }  
}

void SceneOutlineItem::Destroy()
{
    if (!_gameObject.expired())
    {
        _gameObject.lock()->_editorObject = nullptr;
        _gameObject.lock()->Destroy();
    }
    delete this;
}

QVariant SceneOutlineItem::data(int column, int role) const
{
    //if (!_level.expired() && role == Qt::DecorationRole)
    //{
    //    return QIcon(_iconPath);
    //}
    return QTreeWidgetItem::data(column, role);
}

class SceneOutlineTreeDelegate : public QStyledItemDelegate
{
public:
    class SceneOutlineTree* _tree;
    SceneOutlineTreeDelegate(SceneOutlineTree* parent) :QStyledItemDelegate(parent)
    {
        _tree = parent;
    }
    
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        SceneOutlineItem* item = (SceneOutlineItem*)_tree->itemFromIndex(index);
        auto rect = opt.rect;
        if (item)
        {
            if (!item->_level.expired())
            {
                if (item->_level.lock()->IsLoaded())
                {
                    opt.font.setStrikeOut(false);
                    opt.font.setBold(true);
                }
                else// ����û���ص�ʱ����ʾ��ȥЧ��
                {
                    opt.font.setStrikeOut(true);
                    opt.font.setBold(false);
                }
                opt.rect.setX(rect.x() + rect.height());
            }
            else
            {
                opt.font.setStrikeOut(false);
                opt.font.setBold(false);
            }
        }
        QStyledItemDelegate::paint(painter, opt, index);
        if (item) 
        {
            if (!item->_level.expired())
            {
                if (item->_level.lock()->IsLoaded())
                {
                    QPixmap eye((FileSystem::GetConfigAbsPath() + "Theme/Icons/ICON_SCENE_SHOW.png").c_str());
                    eye.scaled(rect.height()-1, rect.height()-1);
                    painter->drawPixmap(rect.x(), rect.y(), rect.height(), rect.height(), eye);
                }
                else
                {
                    QPixmap eye((FileSystem::GetConfigAbsPath() + "Theme/Icons/ICON_SCENE_HIDE.png").c_str());
                    eye.scaled(rect.height()-1, rect.height()-1);
                    painter->drawPixmap(rect.x(), rect.y(), rect.height(), rect.height(), eye);
                }
            }
        }
    }
};

SceneOutlineTree::SceneOutlineTree(QWidget* parent)
    :QTreeWidget(parent)
{
    _parent = (SceneOutline*)parent;
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    setIconSize({ 0,0 });
    //setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    //�����Ϸ�ģʽΪ�ڲ��ƶ�
    setDragDropMode(QAbstractItemView::InternalMove);
    //�������drop����
    setAcceptDrops(true);
    setEditTriggers(EditTrigger::NoEditTriggers);//�ص�˫�������Ĺ���
    //setMouseTracking(true);

    setObjectName("SceneOutline");
    viewport()->setObjectName("SceneOutline");

    setIndentation(10);
    setHeaderHidden(true);
    //setColumnCount(2);
    //setHeaderLabels({
    //    "",
    //    "Label" });

     setItemDelegate(new SceneOutlineTreeDelegate(this));

     _menu = new QMenu(this);
     _menu_createBasic = new QMenu(this);

    //setRootIsDecorated(false);
    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),this,SLOT(ItemDoubleClicked(QTreeWidgetItem*, int)));
    connect(this, SIGNAL(sigEditFinished(QString)), this, SLOT(ItemEditFinished(QString)));
    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(ItemSelectionChanged()));
   
    //��Item�����仯
    connect(this, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* item, int column)
        {
            
        });
}

SceneOutlineItem* SceneOutlineTree::IndexToItem(QModelIndex index)
{
    return (SceneOutlineItem*)this->itemFromIndex(index);
}

void SceneOutlineTree::contextMenuEvent(QContextMenuEvent* event)
{
    _menu->clear();
    _menu_createBasic->clear();

    _createNewGameObject = new QAction(GetEditorInternationalization("SceneOutline", "CreateNewGameObject"), _menu);
    _deleteGameObject = new QAction(GetEditorInternationalization("SceneOutline", "DeleteGameObject"), _menu);
    _renameGameObject = new QAction(GetEditorInternationalization("SceneOutline", "RenameGameObject"), _menu);

    _menu_createBasic->setTitle(GetEditorInternationalization("SceneOutline", "CreateBasicGameObject"));
    _createCube = new QAction(GetEditorInternationalization("SceneOutline", "CreateCube"), _menu_createBasic);
    _createSphere = new QAction(GetEditorInternationalization("SceneOutline", "CreateSphere"), _menu_createBasic);
    _createPlane = new QAction(GetEditorInternationalization("SceneOutline", "CreatePlane"), _menu_createBasic);

    _createNewLevel = new QAction(GetEditorInternationalization("SceneOutline", "CreateNewLevel"), _menu_createBasic);
    _deleteLevel = new QAction(GetEditorInternationalization("SceneOutline", "DeleteLevel"), _menu_createBasic);
    _renameLevel = new QAction(GetEditorInternationalization("SceneOutline", "RenameLevel"), _menu_createBasic);
    _loadLevel = new QAction(GetEditorInternationalization("SceneOutline", "LoadLevel"), _menu_createBasic);
    _unloadLevel = new QAction(GetEditorInternationalization("SceneOutline", "UnloadLevel"), _menu_createBasic);
    //
    connect(_createNewGameObject, &QAction::triggered, this, [this](bool bChecked)
        {
            if (VulkanApp::GetMainForm()->renderer)
            {
                VulkanApp::GetMainForm()->renderer->ExecFunctionOnRenderThread([]()
                    {
                        GameObject::CreateGameObject();
                    });
            }
            ((SceneOutlineItem*)this->currentItem())->_gameObject.lock()->GetLevel()->MarkDirty();
        });
    connect(_renameGameObject, &QAction::triggered, this, [this](bool bChecked)
        {
            if (this->currentItem())
                editItem(this->currentItem());
        });
    connect(_deleteGameObject, &QAction::triggered, this, [this](bool bChecked)
        {
            VulkanApp::GetMainForm()->renderer->ExecFunctionOnRenderThread([this]()
                {
                    if (this->currentItem())
                    {
                        ((SceneOutlineItem*)this->currentItem())->_gameObject.lock()->Destroy();
                    }
                });
            ((SceneOutlineItem*)this->currentItem())->_gameObject.lock()->GetLevel()->MarkDirty();
        });
    connect(_createCube, &QAction::triggered, this, [this](bool bChecked)
        {
            VulkanApp::GetMainForm()->renderer->ExecFunctionOnRenderThread([]()
                {
                    QFileInfo fi("./Asset/Content/Core/Basic/Cube");
                    //GameObject::CreateModelGameObject(fi.absolutePath().toStdString().c_str());
                });
            ((SceneOutlineItem*)this->currentItem())->_gameObject.lock()->GetLevel()->MarkDirty();
        });
    connect(_createSphere, &QAction::triggered, this, [this](bool bChecked)
        {
            VulkanApp::GetMainForm()->renderer->ExecFunctionOnRenderThread([]()
                {
                    QFileInfo fi("./Asset/Content/Core/Basic/Sphere");
                    //GameObject::CreateModelGameObject(fi.absolutePath().toStdString().c_str());
                });
            ((SceneOutlineItem*)this->currentItem())->_gameObject.lock()->GetLevel()->MarkDirty();
        });
    connect(_createPlane, &QAction::triggered, this, [this](bool bChecked)
        {
            VulkanApp::GetMainForm()->renderer->ExecFunctionOnRenderThread([]()
                {
                    QFileInfo fi("./Asset/Content/Core/Basic/Plane");
                    //GameObject::CreateModelGameObject(fi.absolutePath().toStdString().c_str());
                });
            ((SceneOutlineItem*)this->currentItem())->_gameObject.lock()->GetLevel()->MarkDirty();
        });
    //�����³���
    connect(_createNewLevel, &QAction::triggered, this, [this](bool bChecked)
        {
            HString newLevelName = "New Level";
            int index = 0;
            while (true)
            {
                bool bFound = false;
                for (auto& i : _parent->currentWorld.lock()->GetLevels())
                {
                    if (i->GetLevelName().IsSame(newLevelName))
                    {
                        bFound = true;
                        newLevelName = "New Level " + HString::FromInt(index);
                        index++;
                        break;
                    }
                }
                if (!bFound)
                    break;
            }
            _parent->currentWorld.lock()->AddNewLevel(newLevelName);
        });
    //ɾ������
    connect(_deleteLevel, &QAction::triggered, this, [this](bool bChecked)
        {
            auto manager = new DirtyAssetsManager(this);
            if (manager)
            {
                manager->setWindowTitle(GetEditorInternationalization("DirtyAssetsManager", "BaseTitle1"));
                auto func =
                    [this, &manager]() {
                    manager = nullptr;
                    };
                manager->_finishExec.push_back(func);
                if (manager->exec() == -10)
                {
                    manager = nullptr;
                }
            }
            if (!manager)
            {
                //��ж�س���
                for (auto& i : GetSelectionLevels())
                {
                    i.lock()->UnLoad();
                }
                //�����༭����
                std::vector<SceneOutlineItem*> items;
                for (auto& i : _parent->_treeWidget->selectedItems())
                {
                    items.push_back((SceneOutlineItem*)_parent->_treeWidget->takeTopLevelItem(_parent->_treeWidget->indexOfTopLevelItem(i)));
                }
                if (_parent->_currentLevelItem != nullptr)
                {
                    if (_parent->_comboBox->GetCurrentSelection().compare(_parent->_currentLevelItem->_level.lock()->GetLevelName().c_str(), Qt::CaseInsensitive))
                    {
                        _parent->ClearCurrentLevelSelection();
                        for (auto& i : _parent->_levelItems)
                        {
                            if (i->_level.lock()->IsLoaded())
                            {
                                _parent->SetCurrentLevelSelection(i);
                                break;
                            }
                        }
                    }
                }          
                for (auto& i :items)
                {
                    delete i;
                }
                items.clear();
                // ((SceneOutlineItem*)this->currentItem())->_gameObject.lock()->GetLevel()->MarkDirty();
                std::vector<HString> levelName;
                for (auto& i : GetSelectionLevels())
                {
                    _parent->currentWorld.lock()->DeleteLevel(i.lock()->GetLevelName());
                }
                _parent->currentWorld.lock()->MarkDirty();
            }
        });
    //����������
    connect(_renameLevel, &QAction::triggered, this, [this](bool bChecked)
        {
            if (this->currentItem())
                editItem(this->currentItem());
        });
    //��������
    connect(_loadLevel, &QAction::triggered, this, [this](bool bChecked)
        {
            for (auto& i : GetSelectionLevels())
            {
                i.lock()->Load();
            }
        });
    //����ж��
    connect(_unloadLevel, &QAction::triggered, this, [this](bool bChecked)
        {
            auto manager = new DirtyAssetsManager(this);
            if (manager)
            {
                auto func =
                    [this, &manager]() {
                    manager = nullptr;
                    };
                manager->_finishExec.push_back(func);
                if (manager->exec() == -10)
                {
                    manager = nullptr;
                }
            }
            if (!manager)
            {
                for (auto& i : GetSelectionLevels())
                {
                    i.lock()->UnLoad();
                }
                //�����༭����
                if (_parent->_currentLevelItem)
                {
                    if (_parent->_comboBox->GetCurrentSelection().compare(_parent->_currentLevelItem->_level.lock()->GetLevelName().c_str(), Qt::CaseInsensitive))
                    {
                        _parent->ClearCurrentLevelSelection();
                        for (auto& i : _parent->_levelItems)
                        {
                            if (i->_level.lock()->IsLoaded())
                            {
                                _parent->SetCurrentLevelSelection(i);
                                break;
                            }
                        }
                    }
                }
                else
                {
                    _parent->ClearCurrentLevelSelection();
                }
            }
        });
    //
    auto item = (SceneOutlineItem*)itemAt(event->pos());
    if (item)
    {
        if (!item->_gameObject.expired())
        {
            _menu_createBasic->addAction(_createCube);
            _menu_createBasic->addAction(_createSphere);
            _menu_createBasic->addAction(_createPlane);

            _menu->addAction(_createNewGameObject);
            _menu->addMenu(_menu_createBasic);
            _menu->addAction(_renameGameObject);
            _menu->addSeparator();
            _menu->addAction(_deleteGameObject);
        }
        else if (!item->_level.expired())
        {
            _menu->addAction(_createNewLevel);
            _menu->addAction(_renameLevel);
            _menu->addAction(_loadLevel);
            _menu->addAction(_unloadLevel);
            _menu->addSeparator();
            _menu->addAction(_deleteLevel);
        }
        _menu->exec(event->globalPos());
    }
}

void SceneOutlineTree::ItemSelectionChanged()
{
    auto objects = GetSelectionObjects();
    if (Inspector::_currentInspector != nullptr)
    {
        if (objects.size() > 0)
        {
            if (Inspector::_currentInspector != nullptr)
                Inspector::_currentInspector->LoadInspector_GameObject(objects[0].lock()->GetSelfWeekPtr());
        }
        else
        {
            Inspector::_currentInspector->ClearInspector();
        }
    }
}

void SceneOutlineTree::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        _curItem = this->itemAt(event->pos());
        if (_curItem == nullptr)
        {
            this->clearSelection();
        }
        else
        {
            _mouse_select_item = (SceneOutlineItem*)_curItem;
        }
    }
    QTreeWidget::mousePressEvent(event);
}

void SceneOutlineTree::mouseMoveEvent(QMouseEvent* event)
{
    _mouseTouchItem = this->itemAt(event->pos());
    QTreeWidget::mouseMoveEvent(event);
}

void SceneOutlineTree::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))//���������Item������
    {
        //QT ��Item��ֻ֧������ContentBrowser����ק
        QObject* par = e->source();
        if (par->objectName().contains("SceneOutline", Qt::CaseInsensitive))
        {
            e->acceptProposedAction();
        }
    }
}

void SceneOutlineTree::dropEvent(QDropEvent* event)
{
    QTreeWidget::dropEvent(event);
    if (_curItem != nullptr)
    {
        SceneOutlineItem* dragItem = (SceneOutlineItem*)_curItem;
        if (_mouseTouchItem == nullptr)
        {
            if(!dragItem->_gameObject.expired())
                dragItem->_gameObject.lock()->SetParent(nullptr);
        }
        else
        {
            SceneOutlineItem* currentItem = (SceneOutlineItem*)(_mouseTouchItem);
            if(!dragItem->_gameObject.expired())
            {
                //GameObject
                if (dragItem->parent())
                {
                    auto newParent = ((SceneOutlineItem*)(dragItem->parent()))->_gameObject;
                    if (!newParent.expired())
                    {
                        //��������ͬ,�ȸ�������
                        if (dragItem->_gameObject.lock()->GetLevel()->GetGUID() != newParent.lock()->GetLevel()->GetGUID())
                        {
                            dragItem->_gameObject.lock()->ChangeLevel(newParent.lock()->GetLevel()->GetLevelName());
                            ConsoleDebug::printf_endl(GetEditorInternationalization("SceneOutline","SetGameObjectLevel").toStdString().c_str(), dragItem->_gameObject.lock()->GetObjectName().c_str(), newParent.lock()->GetLevel()->GetLevelName().c_str());
                        }
                        dragItem->_gameObject.lock()->SetParent(newParent.lock().get());
                        ConsoleDebug::printf_endl(GetEditorInternationalization("SceneOutline", "SetGameObjectParent").toStdString().c_str(), dragItem->_gameObject.lock()->GetObjectName().c_str(), newParent.lock()->GetObjectName().c_str());
                    }
                }
                else
                {
                    dragItem->_gameObject.lock()->SetParent(nullptr);
                    ConsoleDebug::printf_endl(GetEditorInternationalization("SceneOutline", "SetGameObjectParent").toStdString().c_str(), dragItem->_gameObject.lock()->GetObjectName().c_str(), "null");
                }

                //Level
                auto newLevelItem = ((SceneOutlineItem*)(dragItem->parent()))->_level;
                if (!newLevelItem.expired())
                {
                    if (!newLevelItem.lock()->IsLoaded())
                    {
                        ConsoleDebug::printf_endl(GetEditorInternationalization("SceneOutline", "SetGameObjectLevelFailed").toStdString().c_str(), newLevelItem.lock()->GetLevelName().c_str());
                        event->ignore();
                    }
                    else
                    {
                        dragItem->_gameObject.lock()->ChangeLevel(newLevelItem.lock()->GetLevelName());
                        ConsoleDebug::printf_endl(GetEditorInternationalization("SceneOutline", "SetGameObjectLevel").toStdString().c_str(), dragItem->_gameObject.lock()->GetObjectName().c_str(), newLevelItem.lock()->GetLevelName().c_str());
                    }
                }
            }
        }
        _curItem = nullptr;
    }
}

QList<std::weak_ptr<GameObject>> SceneOutlineTree::GetSelectionObjects()
{
    QList<std::weak_ptr<GameObject>> result;
    for (auto i : this->selectedItems())
    {
        auto item = dynamic_cast<SceneOutlineItem*>(i);
        if (item != nullptr && !item->_gameObject.expired())
        {
            result.append(item->_gameObject);
        }
    }
    return result;
}

QList<std::weak_ptr<Level>> SceneOutlineTree::GetSelectionLevels()
{
    QList<std::weak_ptr<Level>> result;
    for (auto i : this->selectedItems())
    {
        auto item = dynamic_cast<SceneOutlineItem*>(i);
        if (item != nullptr && !item->_level.expired())
        {
            result.append(item->_level);
        }
    }
    return result;
}

void SceneOutlineTree::ItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    if (!item->isExpanded())
    {
        collapseItem(item);
    }
    else
    {
        expandItem(item);
    }
}

void SceneOutlineTree::ItemEditFinished(QString newText)
{
    SceneOutlineItem* objItem = (SceneOutlineItem*)currentItem();
    if (!objItem->_gameObject.expired() && objItem->_gameObject.lock()->GetObjectName() != newText.toStdString().c_str())
    {
        objItem->_gameObject.lock()->SetObjectName(newText.toStdString().c_str());
        objItem->_gameObject.lock()->GetLevel()->MarkDirty();
    }
    else if (!objItem->_level.expired() && !objItem->_level.lock()->GetLevelName().IsSame(newText.toStdString().c_str()))
    {
        objItem->_level.lock()->Rename(newText.toStdString().c_str());
        objItem->_level.lock()->MarkDirty();
        _parent->currentWorld.lock()->MarkDirty();
    }
}

SceneOutline::SceneOutline(VulkanRenderer* renderer, QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setObjectName("SceneOutline");
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(1,1,1,1);
    mainLayout->setSpacing(1);
    this->setLayout(mainLayout);

    _search = new CustomSearchLine(this);
    _search->setMaximumHeight(30);
    _search->ui.comboBox->setHidden(true);
    mainLayout->addWidget(_search);
    connect(_search->ui.lineEdit, SIGNAL(returnPressed()), this, SLOT(TreeSearch()));

    _treeWidget = new SceneOutlineTree(this);
    mainLayout->addWidget(_treeWidget);

    //Level selection
    _comboBox = new ComboBox(GetEditorInternationalization("SceneOutline", "LevelSelection"), this);
    _comboBox->setObjectName("SceneOutline_ComboBox_Main");
    _comboBox->ui.ComboBox_0->setObjectName("SceneOutline_ComboBox");
    _comboBox->ui.Name->setObjectName("SceneOutline_Name");
    _comboBox->ui.horizontalLayout->setStretch(0, 0);
    _comboBox->ui.horizontalLayout->setStretch(1, 0); 
    //����ComboBox�ص�
    _comboBox->_bindCurrentTextChanged =
        [this](const int index, const char* text)
    {
        for (auto& i : _levelItems)
        {
            if (!i->_level.expired())
            {
                if (i->_level.lock()->GetLevelName().IsSame(text))
                {
                    _currentLevelItem = i;
                    ConsoleDebug::printf_endl(GetEditorInternationalization("SceneOutline", "CurrentEditLevelLog").toStdString().c_str(), i->_level.lock()->GetLevelName().c_str());
                    _treeWidget->expandItem(_currentLevelItem);
                    i->_level.lock()->Load();
                    break;
                }
            }
        }
    };
    mainLayout->addWidget(_comboBox);

    _renderer = renderer;
    //World���ɵ�ʱ��ִ��
    auto spawnNewWorldCallBack = [this](std::weak_ptr<World> world)
    {
        if (!world.expired() && _renderer->IsMainRenderer())
        {
            currentWorld = world;
            //Worldÿ֡����
            world.lock()->_editorWorldUpdate=
                [](std::vector<std::weak_ptr<Level>>&levels)
                {
                    
                };
            //Worldж�ص�ʱ��ִ��
            world.lock()->_editorWorldRelease =
                [this, world]()
                {
                    for (auto& i : _levelItems)
                    {
                        auto level = _treeWidget->takeTopLevelItem(_treeWidget->indexOfTopLevelItem(i));
                        delete level;
                    }
                    _levelItems.clear();
                    //��������Ԥ��ͼ


                };

            //World��Level���鷢���仯��ʱ��ִ��(���� ��������ɾ��Level)
            world.lock()->_editorLevelChanged =
                [this, world]()
                {
                    for (auto& i : _levelItems)
                    {
                        auto level = _treeWidget->takeTopLevelItem(_treeWidget->indexOfTopLevelItem(i));
                        delete level;
                    }
                    _levelItems.clear();
                    if (!world.expired())
                    {
                        for (auto& i : world.lock()->GetLevels())
                        {
                            auto itemExist = FindLevel(i->GetGUID());
                            if (itemExist == nullptr)
                            {
                                //����LevelĿ¼
                                auto item = new SceneOutlineItem(i, std::weak_ptr<GameObject>(), (FileSystem::GetConfigAbsPath() + "Theme/Icons/ICON_SCENE.png").c_str(), _treeWidget);
                                _treeWidget->addTopLevelItem(item);
                                _levelItems.insert(i->GetLevelName().c_str(), item);

                                auto iconPath = FileSystem::Append(FileSystem::GetConfigAbsPath(), "Theme/Icons/ICON_SCENE.png");
                                QIcon icon(iconPath.c_str());
                                item->setIcon(0, icon);
                            }
                            ResetLevelSelectionComboBox();
                        }
                        EditorMain::_self->setWindowTitle(GetEditorInternationalization("MainWindow", "MainTitle") + " - " + world.lock()->GetWorldName().c_str());
                    }
                };

            world.lock()->_editorLevelVisibilityChanged =
                [this, world](Level* level , bool bVisibility)
            {
                if (level)
                {
                    auto levelItem = FindLevel(level->GetLevelName().c_str());
                }
            };

            //�༭���� GameObject ����ί��
            world.lock()->_editorGameObjectAddFunc = [this]
            (std::shared_ptr<GameObject> object)
            {
                QString levelName = object->GetLevel()->GetLevelName().c_str();
                auto levelItem = FindLevel(levelName);
                if (levelItem)
                {
                    //�ڶ�Ӧ��LevelĿ¼�����Item
                    auto newObjectItem = new SceneOutlineItem(std::weak_ptr<Level>(), object, levelItem);
                }
            };

            //�༭���� GameObject SetParentί��
            world.lock()->_editorGameObjectSetParentFunc = [this]
            (std::shared_ptr<GameObject> object, std::shared_ptr<GameObject> newParent)
            {
                    if (newParent && object->_editorObject && newParent->_editorObject)
                    {
                        SceneOutlineItem* child = (SceneOutlineItem*)object->_editorObject;
                        SceneOutlineItem* parent = (SceneOutlineItem*)newParent->_editorObject;
                        child->parent()->takeChild(child->parent()->indexOfChild(child));
                        parent->addChild(child);
                    }
            };
            

            //�༭���� GameObjectÿ֡����
            world.lock()->_editorGameObjectUpdateFunc = [this]
            (std::shared_ptr<GameObject> object)
            {
                if (object->_bEditorNeedUpdate)
                {
                    object->_bEditorNeedUpdate = false;
                    auto objects = SceneOutline::_treeWidget->GetSelectionObjects();
                    if (objects.size() > 0)
                    {
                        if (Inspector::_currentInspector != nullptr)
                            Inspector::_currentInspector->LoadInspector_GameObject(objects[0].lock()->GetSelfWeekPtr(), true);
                    }
                    if (object->_editorObject)
                    {
                        auto item = (SceneOutlineItem*)object->_editorObject;
                        //rename?
                        item->setText(0, object->GetObjectName().c_str());
                    }
                }
            };

            //�༭���� GameObject ����ί��
            world.lock()->_editorGameObjectRemoveFunc = [this]
            (std::shared_ptr<GameObject> object)
            {
                auto item = (SceneOutlineItem*)object->_editorObject;
                //QT5 QTreeWidgetItem ɾ���˸��ڵ�,�ӽڵ��ڴ�Ҳ�����һ������,
                //�ᵼ��object->_editorObject(�ڴ��ѱ�ɾ��)��ȡ�����쳣,
                //�������ٽڵ�֮ǰ�Ȱ��ӽڵ�ȡ������ֹ��ͻ��
                if (item != nullptr)
                {
                    if (item->childCount() > 0)
                    {
                        if (item->parent())
                        {
                            item->parent()->addChildren(item->takeChildren());
                        }
                        else
                        {
                            _treeWidget->addTopLevelItems(item->takeChildren());
                        }
                    }
                    item->Destroy();
                }
            };
            
            for (auto &i : world.lock()->GetLevels())
            {

            }
        }
    }; 
    _renderer->_spwanNewWorld.push_back(spawnNewWorldCallBack);

}

SceneOutline::~SceneOutline()
{
}

void SceneOutline::closeEvent(QCloseEvent* event)
{

}

void traverse(QTreeWidgetItem* item , QList<QTreeWidgetItem*>& list , bool bHide = false , QString findName = "")
{
    // ���������ӽڵ�
    for (int i = 0; i < item->childCount(); ++i) {
        traverse(item->child(i) , list , bHide);
        if (findName.length() > 0)
        {
            if (item->child(i)->text(0).contains(findName))
            {

            }
        }
        list.append(item->child(i));
        item->child(i)->setHidden(bHide);
    }
    item->setHidden(bHide);
}

bool searchTraverse(QTreeWidgetItem* item, QList<QTreeWidgetItem*>& list, QString findName = "")
{
    bool bHide = true;
    // ���������ӽڵ�
    for (int i = 0; i < item->childCount(); ++i) {
        bHide = bHide && searchTraverse(item->child(i), list);
        if (findName.length() > 0)
        {
            if (item->child(i)->text(0).contains(findName))
            {
                bHide = false;
                continue;
            }
        }
        item->child(i)->setHidden(bHide);
        list.append(item->child(i));
    }

    if (findName.length() > 0)
    {
        if (item->text(0).contains(findName))
        {
            return false;
        }
    }

    return bHide;
}

void SceneOutline::TreeSearch()
{
    //_treeWidget->collapseAll();
    QList<QTreeWidgetItem*> searchList;
    _treeWidget->selectionModel()->clearSelection();
    if (_search->ui.lineEdit->text().isEmpty())
    {
        for (int i = 0; i < _treeWidget->topLevelItemCount(); ++i) {
            traverse(_treeWidget->topLevelItem(i), searchList, false);
        }
        return;
    }
    QString input = _search->ui.lineEdit->text();
    _treeWidget->expandAll();
    //auto searchList = _treeWidget->findItems(input, Qt::MatchExactly)
    for (int i = 0; i < _treeWidget->topLevelItemCount(); ++i) {
        bool bHide = searchTraverse(_treeWidget->topLevelItem(i) , searchList , input);
        _treeWidget->topLevelItem(i)->setHidden(bHide);       
    }
}

void SceneOutline::focusInEvent(QFocusEvent* event)
{

}

void SceneOutline::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QStyleOption styleOpt;
    styleOpt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);
    QWidget::paintEvent(event);
}

void SceneOutline::ClearCurrentLevelSelection()
{
    _currentLevelItem = nullptr;
    _comboBox->SetCurrentSelection("   ");
}

void SceneOutline::SetCurrentLevelSelection(SceneOutlineItem* item)
{
    if (item)
    {
        _currentLevelItem = item;
        _comboBox->SetCurrentSelection(item->_level.lock()->GetLevelName().c_str());
    }
}

void SceneOutline::ResetLevelSelectionComboBox()
{
    _comboBox->ClearItems();

    for (auto& i : _levelItems)
    {
        if (!i->_level.expired())
        {
            _comboBox->AddItem(i->_level.lock()->GetLevelName().c_str() , "   ");
            ////Ĭ�Ͽ�����һ��level�ı༭�ͼ��أ������
            //if (_currentLevelItem == nullptr && _levelItems.size() > 0)
            //{
            //    _currentLevelItem = FindLevel(i->_level.lock()->GetGUID());
            //    _comboBox->SetCurrentSelection(i->_level.lock()->GetLevelName().c_str());
            //}
        }
    }
    if (_currentLevelItem == nullptr && _levelItems.size() <= 0)
    {
        ClearCurrentLevelSelection();
    }
}

SceneOutlineItem* SceneOutline::FindLevel(QString levelName)
{
    SceneOutlineItem* result = nullptr; 
    auto it = _levelItems.find(levelName);
    if (it != _levelItems.end())
    {
        result = it.value();
    }
    return result;
}

SceneOutlineItem* SceneOutline::FindLevel(HGUID guid)
{
    SceneOutlineItem* result = nullptr;
    for (auto& i : _levelItems)
    {
        if (!i->_level.expired())
        {
            if (i->_level.lock()->GetGUID() == guid)
            {
                result = i;
                break;
            }
        }
    }
    return result;
}

