#include "EditorMain.h"
#include "RenderView.h"
#include "SceneOutline.h"
#include "ContentBrowser.h"
#include "Inspector.h"
#include "FormMain.h"
#include "EditorCommonFunction.h"
#include <QDesktopServices>
#include <QUrl>
#include <qdir.h>
#include <QFileDialog>
#include <qfileinfo.h>
#include "LineEditDialog.h"
#include "DirtyAssetsManager.h"
#include "RendererCore/Core/VulkanRenderer.h"
#include "RendererCore/Core/Asset/World.h"
#include "QFontDatabase.h"
#include "ConsoleDebug.h"
#include <QCloseEvent>
#include <WorldSelector.h>
#include <MaterialDetailEditor.h>
#include "CustomDockPanelTitleBar.h"
#include "CustomTitleBar.h"
#include "PerformanceWatcher.h"
EditorMain* EditorMain::_self = nullptr;

CustomDockWidget::CustomDockWidget(QWidget* parent) :QDockWidget(parent)
{
    auto titleBar = new CustomDockPanelTitleBar(this);
    this->setTitleBarWidget(titleBar);
    titleBar->CloseButtonVisiable(false);
}

EditorMain::EditorMain(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setFocusPolicy(Qt::ClickFocus);

    _mainRenderView = new RenderView(this);
    setCentralWidget(_mainRenderView);

    _sceneOutline = new SceneOutline(_mainRenderView->_mainRendererWidget->_rendererForm->renderer, this);
    _sceneOutline_dock = new CustomDockWidget(this);
    _sceneOutline_dock->setFeatures(
        QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    _sceneOutline_dock->setWidget(_sceneOutline);
    _sceneOutline->setObjectName("SceneOutline");
    _sceneOutline_dock->setWindowTitle(GetEditorInternationalization("MainWindow","SceneOutlineTitle"));
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, _sceneOutline_dock);

    _contentBrowser = new ContentBrowser(this);
    _contentBrowser_dock = new CustomDockWidget(this);
    _contentBrowser_dock->setFeatures(
        QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    _contentBrowser_dock->setWidget(_contentBrowser);
    _contentBrowser->setObjectName("ContentBrowser");
    _contentBrowser_dock->setWindowTitle(GetEditorInternationalization("MainWindow", "ContentBrowserTitle"));
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, _contentBrowser_dock);

    _inspector = new Inspector(this);
    _inspector_dock = new CustomDockWidget(this);
    _inspector_dock->setFeatures(
        QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    _inspector_dock->setWidget(_inspector);
    _inspector->setObjectName("Inspector");
    _inspector_dock->setWindowTitle(GetEditorInternationalization("MainWindow", "Inspector"));
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, _inspector_dock);

    setObjectName("EditorMain");
    //setStyleSheet(GetWidgetStyleSheetFromFile("EditorMain"));
    connect(ui.PerformanceWatcher, &QAction::triggered, this, [this](bool bChecked) {
        PerformanceWatcher* mw = new PerformanceWatcher(this);
        mw->show();
    });
    connect(ui.ResetWindowStyle, &QAction::triggered, this, [this](bool bChecked) {
        if (this->parentWidget() != nullptr)
        {
            this->parentWidget()->setStyleSheet(GetWidgetStyleSheetFromFile("BEGIN"));
        }
        setStyleSheet(GetWidgetStyleSheetFromFile("BEGIN"));
    });
    connect(ui.OpenProgramDir, &QAction::triggered, this, [this](bool bChecked) {
        //QDir dir(FileSystem::GetProgramPath().c_str());
        QFileInfo info(QDir::toNativeSeparators(FileSystem::GetProgramPath().c_str()));
        if (info.isDir())
        {
            QDesktopServices::openUrl(QUrl(info.absoluteFilePath()));
        }
    });
    connect(ui.CompileAllShader, &QAction::triggered, this, [this](bool bChecked) {
        VulkanApp::RecompileAllShader();
        MaterialDetailEditor::RefreshAllEditor();
    });
    connect(ui.CreateNewWorld, &QAction::triggered, this, [this](bool bChecked) {
        VulkanApp::GetMainForm()->renderer->CreateEmptyWorld();
    });
    connect(ui.SaveWorld, &QAction::triggered, this, [this](bool bChecked) {
        VulkanApp::GetMainForm()->renderer->GetWorld().lock()->SaveWorld();
     });
    connect(ui.SaveAsWorld, &QAction::triggered, this, [this](bool bChecked) {
        LineEditDialog* dialog = new LineEditDialog("New World Name",this);
        dialog->EnterCallBack = [dialog]() {
            if (dialog->ui.lineEdit->text().size() > 1)
            {
                VulkanApp::GetMainForm()->renderer->GetWorld().lock()->SaveWorld(dialog->ui.lineEdit->text().toStdString().c_str());
            }
        };       
    });

    EditorMain::_self = this;

    //Update timer //�༭�����Ŵ��ڵ�ʱ����Ⱦ���᲻ͣ���ڣ�
    //������Ϊ�������ź���Ⱦ����(Update)��ͬ�����£������Ѿ����ŵ��ǻ���δ��ʱ���»��Ƶ���һ˲�䷢��
    //�������ҵ�QT5��Ϣѭ���ĺ�����дӦ�þ��ܽ����������ˡ�
    _renderTimer = new QTimer(this);
    _renderTimer->setInterval(1);
    connect(_renderTimer, SIGNAL(timeout()), this, SLOT(UpdateRender()));
    _renderTimer->start();

    //������
    ActionConnect(ui.OpenWorld, [this]()
        {
            WorldSelector* ws = new WorldSelector(this);
            ws->exec();
        });
    LoadEditorWindowSetting(this, "MainWindow");
}

EditorMain::~EditorMain()
{
}

void EditorMain::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
}

void EditorMain::focusInEvent(QFocusEvent* event)
{
    QMainWindow::focusInEvent(event);
}

void EditorMain::focusOutEvent(QFocusEvent* event)
{
    QMainWindow::focusOutEvent(event);
}

void EditorMain::UpdateRender()
{
    VulkanApp::UpdateForm();
    if (_inspector)
        _inspector->PropertyUpdate();
    if (_mainRenderView)
        _mainRenderView->Update();
}

DirtyAssetsManager* EditorMain::ShowDirtyAssetsManager()
{
    if (ContentManager::Get()->GetDirtyAssets().size() + World::GetDirtyWorlds().size() + Level::GetDirtyLevels().size() > 0)
    {
        auto dirtyAssetsManager = new DirtyAssetsManager(this);
        {
            dirtyAssetsManager->exec();
            return dirtyAssetsManager;
        }
    } 
    return nullptr;
}

void EditorMain::closeEvent(QCloseEvent* event)
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
    if(!manager)
    {
        _renderTimer->stop();
        if (_inspector)
            _inspector->close();
        if (_sceneOutline)
            _sceneOutline->close();
        if (_mainRenderView)
            _mainRenderView->close();
        event->accept();
        if (_customTitleBar)
        {
            this->setParent(nullptr);//��ֹѭ��close
            _customTitleBar->close();
            SaveEditorWindowSetting(_customTitleBar, "MainWindow");
        }
        else
        {
            SaveEditorWindowSetting(this, "MainWindow");
        }
    }
    else
    {
        event->ignore();
    }
}

void EditorMain::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
}
